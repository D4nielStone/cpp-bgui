#include <bgui.hpp>
#include <gtest/gtest.h>
#include <os/asset_manager.hpp>

#include <stdexcept>

TEST(IconTest, ResolvesNamedNerdFontGlyph) {
    const bgui::icon folder{"folder"};

    EXPECT_EQ(folder.type, "icon");
    EXPECT_EQ(folder.get_buffer(), "\xef\x81\xbb");
    ASSERT_TRUE(folder.style.visual.font.has_value());
    EXPECT_EQ(*folder.style.visual.font, "nerd_font");
}

TEST(IconTest, RejectsUnknownName) {
    EXPECT_THROW((bgui::icon{"not_an_icon"}), std::invalid_argument);
}

TEST(IconTest, ResolvesLinuxAndWindowsPlatformGlyphs) {
    const bgui::icon l{"linux"};
    const bgui::icon windows{"windows"};

    EXPECT_EQ(l.get_buffer(), "\xef\x85\xbc");
    EXPECT_EQ(windows.get_buffer(), "\xef\x85\xba");
}

TEST(IconTest, CentersVisibleGlyphInsideButtonAtDifferentScales) {
    bgui::scoped_interface interface;
    bgui::font font;
    font.ascent = 32.f;
    font.descent = 8.f;
    font.line_gap = 4.f;
    bgui::character glyph;
    glyph.size = {18, 16};
    glyph.bearing = {-3, 14};
    glyph.advance = 40;
    font.chs.emplace(0xf015, glyph);
    bgui::asset_manager::get_instance().store_font(
        "icon-centering-test", bgui::font_manager::m_default_resolution,
        std::move(font));

    auto& button = bgui::get_layout().add<bgui::button>("", .3f, [] {});
    button.get_label().set_enable(false);
    button.style.layout.require_size(32.f, 32.f);
    button.style.layout.require_mode(bgui::mode::pixel, bgui::mode::pixel);
    button.style.layout.padding = bgui::vec4i{0};
    button.style.layout.align = bgui::vec<2, bgui::alignment>{
        bgui::alignment::center, bgui::alignment::center};
    auto& icon = button.add<bgui::icon>("home", .4f);
    icon.style.visual.font = "icon-centering-test";
    icon.style.layout.require_mode(bgui::mode::wrap_content, bgui::mode::wrap_content);
    icon.style.layout.align = bgui::vec<2, bgui::alignment>{
        bgui::alignment::center, bgui::alignment::center};

    for (const float scale : {1.f, 1.5f}) {
        bgui::set_global_scale(scale);
        for (int frame = 0; frame < 3; ++frame)
            bgui::on_update();
        bgui::draw_data data;
        data.m_clip_rect = {0, 0, 900, 600};
        icon.get_requires(&data);
        ASSERT_EQ(data.m_quad_requires.size(), 1u);
        const auto rect = data.m_quad_requires.front().m_rect;
        SCOPED_TRACE(testing::Message() << "scale=" << scale
            << " icon=" << icon.processed_x() << "," << icon.processed_y()
            << " " << icon.processed_width() << "x" << icon.processed_height()
            << " content=" << icon.get_content_size().x << "x" << icon.get_content_size().y);
        EXPECT_NEAR(rect.x + rect.z * .5f,
            button.processed_x() + button.processed_width() * .5f, 1.f);
        EXPECT_NEAR(rect.y + rect.w * .5f,
            button.processed_y() + button.processed_height() * .5f, 1.f);
        EXPECT_FLOAT_EQ(rect.z, 18.f * .4f * scale);
        EXPECT_FLOAT_EQ(-rect.w, 16.f * .4f * scale);
    }
}

TEST(IconTest, ResolvesModelingToolbarGlyphs) {
    for (const auto* name : {"square", "vertices", "edges", "extrude", "redo", "undo",
                             "group", "cut", "material", "orbit", "frame"}) {
        EXPECT_NO_THROW((bgui::icon{name}));
    }
}
