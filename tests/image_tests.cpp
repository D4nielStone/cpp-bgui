#include <gtest/gtest.h>
#include "bgui.hpp"

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
