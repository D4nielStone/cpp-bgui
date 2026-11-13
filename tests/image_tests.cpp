#include <gtest/gtest.h>
#include "bgui.hpp"
#include "os/asset_manager.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>

using namespace bgui;

TEST(ImageTest, UsesNaturalTextureSizeForWrapContent) {
    texture source;
    source.m_size = {320.f, 180.f};
    source.m_buffer = {255, 255, 255, 255};

    image image_element(source);
    image_element.compute_style();
    image_element.calc_content_size(layer::base);
    image_element.process_required_size({800, 600});

    EXPECT_EQ(image_element.processed_width(), 320);
    EXPECT_EQ(image_element.processed_height(), 180);
}

TEST(ImageTest, SupportsExplicitSize) {
    texture source;
    source.m_size = {320.f, 180.f};
    source.m_buffer = {255, 255, 255, 255};

    image image_element(source);
    image_element.set_size(640.f, 360.f);
    image_element.compute_style();
    image_element.calc_content_size(layer::base);
    image_element.process_required_size({800, 600});

    EXPECT_EQ(image_element.processed_width(), 640);
    EXPECT_EQ(image_element.processed_height(), 360);
}

TEST(ImageTest, SupportsParentMatchingMode) {
    texture source;
    source.m_size = {64.f, 64.f};
    source.m_buffer = {255, 255, 255, 255};

    image image_element(source);
    image_element.set_size_mode(mode::match_parent, mode::match_parent);
    image_element.compute_style();
    image_element.calc_content_size(layer::base);
    image_element.process_required_size({500, 300});

    EXPECT_EQ(image_element.processed_width(), 500);
    EXPECT_EQ(image_element.processed_height(), 300);
}

TEST(ImageTest, FileTexturesUseTopLeftUVAndFramebuffersRemainFlipped) {
    scoped_interface interface;
    texture source;
    source.m_size = {1.f, 2.f};
    source.m_buffer = {255, 0, 0, 255, 0, 0, 255, 255};
    image value(source);
    draw_data calls;
    value.get_requires(&calls);
    ASSERT_FALSE(calls.m_quad_requires.empty());
    EXPECT_EQ(calls.m_quad_requires.front().m_uv_min, (vec2{0.f, 0.f}));
    EXPECT_EQ(calls.m_quad_requires.front().m_uv_max, (vec2{1.f, 1.f}));
    calls.m_quad_requires.pop();
    value.set_external_texture(123, {1.f, 2.f});
    value.get_requires(&calls);
    EXPECT_EQ(calls.m_quad_requires.front().m_uv_min, (vec2{0.f, 1.f}));
    calls.m_quad_requires.pop();
    value.set_texture(source);
    value.get_requires(&calls);
    EXPECT_EQ(calls.m_quad_requires.front().m_uv_min, (vec2{0.f, 0.f}));
}

TEST(ImageTest, DraggedWindowPublishesImageDrawRectInSameFrame) {
    scoped_interface interface;
    get_context().m_size = {900, 600};
    get_context().m_input_map.clear();
    auto& window = get_layout().add<bgui::window>("Image", 0.3f, true);
    window.style.layout.require_size(400.f, 300.f);
    texture source;
    source.m_path = "image-drag-regression";
    source.m_size = {1.f, 2.f};
    source.m_buffer = {255, 0, 0, 255, 0, 0, 255, 255};
    auto& content = window.add<image>(source);
    content.set_size_mode(mode::match_parent, mode::match_parent);
    for (int frame = 0; frame < 4; ++frame) on_update();
    for (int step = 0; step < 5; ++step) {
        const auto before = content.processed_rect();
        get_draw_data()->m_quad_requires = {};
        get_draw_data()->m_draw_list.clear();
        window.get_title().set_drag({17, 13});
        on_update();
        EXPECT_EQ(content.processed_x(), before.x + 17);
        EXPECT_EQ(content.processed_y(), before.y + 13);
        bool found = false;
        auto& calls = get_draw_data()->m_quad_requires;
        while (!calls.empty()) {
            const auto& call = calls.front();
            if (call.m_material.m_texture.m_path == source.m_path) {
                found = true;
                EXPECT_EQ(call.m_rect.x, content.processed_x());
                EXPECT_EQ(call.m_rect.y, content.processed_y());
                EXPECT_EQ(call.m_rect.z, content.processed_width());
                EXPECT_EQ(call.m_rect.w, content.processed_height());
            }
            calls.pop();
        }
        EXPECT_TRUE(found);
    }
}

TEST(ImageTest, DecodesJpegRgbGrayscaleAndCmykToRgba) {
    const auto fixtures = std::filesystem::path(__FILE__).parent_path() / "assets";
    for (const auto* name : {"rgb.jpg", "gray.jpeg", "cmyk.jpg", "progressive.jpg"}) {
        SCOPED_TRACE(name);
        const auto& decoded = asset_manager::get_instance().load_texture((fixtures / name).string());
        EXPECT_EQ(decoded.m_size.x, 3.f);
        EXPECT_EQ(decoded.m_size.y, 2.f);
        ASSERT_EQ(decoded.m_buffer.size(), 24u);
        for (size_t i = 0; i < decoded.m_buffer.size(); i += 4) {
            EXPECT_EQ(decoded.m_buffer[i + 3], 255);
            if (std::string(name) == "gray.jpeg") {
                EXPECT_NEAR(decoded.m_buffer[i], 128, 2);
                EXPECT_EQ(decoded.m_buffer[i], decoded.m_buffer[i + 1]);
                EXPECT_EQ(decoded.m_buffer[i], decoded.m_buffer[i + 2]);
            } else {
#ifdef _WIN32
                if (std::string(name) == "cmyk.jpg") {
                    // WIC converts unprofiled CMYK using its default print profile.
                    EXPECT_NEAR(decoded.m_buffer[i], 228, 5);
                    EXPECT_NEAR(decoded.m_buffer[i + 1], 50, 5);
                    EXPECT_NEAR(decoded.m_buffer[i + 2], 43, 5);
                    continue;
                }
#endif
                EXPECT_GT(decoded.m_buffer[i], 240);
                EXPECT_LT(decoded.m_buffer[i + 1], 10);
                EXPECT_LT(decoded.m_buffer[i + 2], 10);
            }
        }
    }
}
TEST(ImageTest, InvalidJpegThrowsInsteadOfTerminatingTheApplication) {
    const auto path = std::filesystem::temp_directory_path() /
        ("bgui-invalid-jpeg-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".jpg");
    { std::ofstream out(path, std::ios::binary); out << "\xff\xd8invalid jpeg"; }
    EXPECT_THROW(asset_manager::get_instance().load_texture(path.string()), std::runtime_error);
    std::filesystem::remove(path);
}

TEST(ImageTest, FramebufferReplacementKeepsLayoutCleanAndUpdatesCachedDraw) {
    image value;
    value.set_external_texture(71, {640.f, 480.f});
    value.compute_style();
    value.clear_layout_dirty();
    draw_data first;
    value.get_cached_requires(&first);
    value.set_external_texture(72, {640.f, 480.f});
    EXPECT_FALSE(value.is_style_dirty());
    EXPECT_FALSE(value.is_layout_dirty());
    draw_data next;
    value.get_cached_requires(&next);
    ASSERT_EQ(next.m_quad_requires.size(), 1u);
    EXPECT_EQ(next.m_quad_requires.front().m_material.m_texture.m_id, 72u);
    value.set_external_texture(72, {800.f, 600.f}, false);
    EXPECT_TRUE(value.is_layout_dirty());
    draw_data resized;
    value.get_cached_requires(&resized);
    EXPECT_EQ(resized.m_quad_requires.front().m_uv_min, (vec2{0.f, 0.f}));
}

TEST(ImageTest, RepeatedExternalFrameDoesNotInvalidateParentGeometryCache) {
    class draw_probe final : public element {
    public:
        int draws{0};
        void get_requires(draw_data* calls) override {
            ++draws;
            element::get_requires(calls);
        }
    };
    layout root;
    auto& sibling = root.add<draw_probe>();
    sibling.set_final_rect(0, 0, 20, 20);
    auto& preview = root.add<image>();
    preview.set_final_rect(20, 0, 40, 30);
    preview.set_external_texture(81, {40.f, 30.f});
    root.cascade_style();
    root.clear_layout_dirty();
    preview.clear_layout_dirty();
    for (int frame = 0; frame < 10; ++frame) {
        preview.set_external_texture(81, {40.f, 30.f});
        draw_data calls;
        root.get_cached_requires(&calls);
        EXPECT_FALSE(root.is_layout_dirty());
        EXPECT_FALSE(preview.is_style_dirty());
    }
    EXPECT_EQ(sibling.draws, 1);
}

TEST(ImageTest, ExternalFlipAndSizeSettersInvalidateOnlyWhenChanged) {
    image value;
    value.set_external_texture(91, {40.f, 30.f}, true);
    value.set_size(40.f, 30.f);
    value.compute_style();
    value.clear_layout_dirty();
    value.set_size(40.f, 30.f);
    value.set_size_mode(mode::pixel, mode::pixel);
    EXPECT_FALSE(value.is_style_dirty());
    EXPECT_FALSE(value.is_layout_dirty());
    value.set_external_texture(91, {40.f, 30.f}, false);
    EXPECT_FALSE(value.is_layout_dirty());
    draw_data calls;
    value.get_cached_requires(&calls);
    EXPECT_EQ(calls.m_quad_requires.front().m_uv_min, (vec2{0.f, 0.f}));
    value.use_natural_size();
    EXPECT_TRUE(value.is_style_dirty());
    value.compute_style();
    value.clear_layout_dirty();
    value.use_natural_size();
    EXPECT_FALSE(value.is_style_dirty());
    EXPECT_FALSE(value.is_layout_dirty());
}

TEST(ImageTest, RepeatedDockResizePublishesPreviewGeometryInTheSameFrame) {
    scoped_interface interface;
    auto& dock = get_layout().add<bgui::dock>();
    dock.style.layout.require_mode(mode::match_parent, mode::match_parent);
    auto& window = dock.add_window("Preview", dock_area::center);
    auto& preview = window.add<image>();
    preview.set_size_mode(mode::match_parent, mode::match_parent);
    preview.set_external_texture(101, {640.f, 480.f});
    for (const float scale : {1.f, 1.5f, 1.f}) {
        set_global_scale(scale);
        for (const vec2i size : {vec2i{900, 600}, vec2i{240, 180}, vec2i{1200, 800}, vec2i{900, 600}}) {
            get_context().m_size = size;
            get_draw_data()->m_quad_requires = {};
            get_draw_data()->m_draw_list.clear();
            on_update();
            bool found = false;
            auto calls = get_draw_data()->m_quad_requires;
            while (!calls.empty()) {
                const auto& call = calls.front();
                if (call.m_material.m_texture.m_id == 101) {
                    found = true;
                    EXPECT_EQ(call.m_rect.x, preview.processed_x());
                    EXPECT_EQ(call.m_rect.y, preview.processed_y());
                    EXPECT_EQ(call.m_rect.z, preview.processed_width());
                    EXPECT_EQ(call.m_rect.w, preview.processed_height());
                    EXPECT_GT(call.m_rect.z, 0.f);
                    EXPECT_GT(call.m_rect.w, 0.f);
                }
                calls.pop();
            }
            EXPECT_TRUE(found);
        }
    }
    set_global_scale(1.f);
}
