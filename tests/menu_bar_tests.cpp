#include <gtest/gtest.h>

#include "bgui.hpp"
#include "os/asset_manager.hpp"
#include "os/font.hpp"
#include "os/os.hpp"

#include <cmath>
#include <utility>

namespace {
    void use_menu_test_font() {
        auto& fonts = bgui::font_manager::get_instance();
        constexpr unsigned int resolution = bgui::font_manager::m_default_resolution;
        if (!fonts.has_font("menu-test", resolution)) {
            bgui::font test_font;
            test_font.ascent = 10.5f;
            test_font.descent = 3.25f;
            test_font.line_gap = 0.2f;
            for (char32_t codepoint = 32; codepoint < 127; ++codepoint) {
                bgui::character glyph;
                glyph.size = codepoint == U' '
                    ? bgui::vec<2, unsigned int>{0, 0}
                    : bgui::vec<2, unsigned int>{9, 16};
                glyph.bearing = {0, 12};
                glyph.advance = 6;
                test_font.chs.emplace(codepoint, glyph);
            }

            bgui::asset_manager::get_instance().store_font(
                "menu-test",
                resolution,
                std::move(test_font)
            );
        }
        fonts.set_default_font("menu-test", resolution);
    }

    void click_at(const bgui::vec2i& position) {
        auto& context = bgui::get_context();
        context.m_mouse_position = position;
        context.m_input_map[bgui::input_key::mouse_left] =
            bgui::input_action::press;
        bgui::on_update();

        context.m_input_map[bgui::input_key::mouse_left] =
            bgui::input_action::release;
        bgui::on_update();

        context.m_input_map[bgui::input_key::mouse_left] =
            bgui::input_action::none;
        bgui::on_update();
    }

    bgui::vec2i center_of(const bgui::element& element) {
        return {
            element.processed_x() + element.processed_width() / 2,
            element.processed_y() + element.processed_height() / 2
        };
    }
}

TEST(MenuBarTest, TextContentSizeRoundsUpFractionalFontMetrics) {
    use_menu_test_font();
    bgui::text label("Save Project", 0.33f);
    label.style.visual.font = "menu-test";
    label.compute_style();
    label.calc_content_size(bgui::layer::base);

    EXPECT_GE(
        label.get_content_size().x,
        25
    );
    EXPECT_GE(
        label.get_content_size().y,
        6
    );

    label.process_required_size({100, 100});
    label.set_final_rect(
        0,
        0,
        label.get_content_size().x,
        label.get_content_size().y
    );
    bgui::draw_data draw_data;
    label.get_requires(&draw_data);
    EXPECT_GE(
        label.get_content_size().x,
        25
    );
    EXPECT_GE(
        label.get_content_size().y,
        6
    );
    while (!draw_data.m_quad_requires.empty()) {
        const auto& glyph = draw_data.m_quad_requires.front();
        EXPECT_GE(glyph.m_rect.x, 0.f);
        EXPECT_LE(
            glyph.m_rect.x + glyph.m_rect.z,
            static_cast<float>(label.processed_width())
        );
        EXPECT_GE(glyph.m_rect.y + glyph.m_rect.w, 0.f);
        EXPECT_LE(
            glyph.m_rect.y,
            static_cast<float>(label.processed_height())
        );
        draw_data.m_quad_requires.pop();
    }
}

TEST(MenuBarTest, ButtonsAndMenuOptionsRunCallbacksOnMouseRelease) {
    bgui::scoped_interface interface;
    use_menu_test_font();
    auto& context = bgui::get_context();
    context.m_size = {800, 600};
    context.m_input_map.clear();
    context.m_last_mouse_left = false;

    auto& root = bgui::get_layout();
    auto& menu_bar = root.add<bgui::menu_bar>(root);
    menu_bar.style.layout.require_mode(
        bgui::mode::match_parent,
        bgui::mode::wrap_content
    );

    int callback_count = 0;
    auto& action_button = menu_bar.add_menu("Action", [&callback_count]() {
        ++callback_count;
    });
    auto& menu_button = menu_bar.add_menu(
        "File",
        0.33f,
        [&callback_count](bgui::context_menu& menu) {
            menu.add_button("Run", [&callback_count]() {
                ++callback_count;
            });
        }
    );

    bgui::on_update();
    EXPECT_GE(
        action_button.processed_width(),
        static_cast<int>(std::ceil(action_button.get_label().get_text_width(
            action_button.get_label().get_buffer())))
    );
    EXPECT_GE(
        action_button.processed_height(),
        static_cast<int>(std::ceil((10.5f + 3.25f + 0.2f) * 0.33f))
    );
    EXPECT_GE(
        action_button.processed_width(),
        action_button.get_label().get_content_size().x
    );
    EXPECT_GE(
        action_button.processed_height(),
        action_button.get_label().get_content_size().y
    );
    click_at(center_of(action_button));
    EXPECT_EQ(callback_count, 1);

    click_at(center_of(menu_button));
    ASSERT_EQ(callback_count, 1);

    bgui::on_update();
    auto& popup_elements = root.get_elements().at(bgui::layer::overlay);
    ASSERT_EQ(popup_elements.size(), 1U);
    auto* popup = dynamic_cast<bgui::linear*>(popup_elements.front().get());
    ASSERT_NE(popup, nullptr);
    auto& options = popup->get_elements().at(bgui::layer::base);
    ASSERT_EQ(options.size(), 1U);
    auto* option_button = dynamic_cast<bgui::button*>(options.front().get());
    ASSERT_NE(option_button, nullptr);
    EXPECT_GE(
        option_button->processed_width(),
        static_cast<int>(std::ceil(option_button->get_label().get_text_width(
            option_button->get_label().get_buffer())))
    );
    EXPECT_GE(
        option_button->processed_height(),
        static_cast<int>(std::ceil((10.5f + 3.25f + 0.2f) * 0.3f))
    );
    EXPECT_GE(
        option_button->processed_width(),
        option_button->get_label().get_content_size().x
    );
    EXPECT_GE(
        option_button->processed_height(),
        option_button->get_label().get_content_size().y
    );
    EXPECT_GE(popup->processed_width(), option_button->processed_width());

    click_at(center_of(*option_button));
    EXPECT_EQ(callback_count, 2);
    EXPECT_FALSE(popup->is_enabled());
}
