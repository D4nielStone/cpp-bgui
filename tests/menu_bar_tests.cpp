#include <gtest/gtest.h>

#include "bgui.hpp"
#include "os/asset_manager.hpp"
#include "os/font.hpp"
#include "os/os.hpp"
#include "os/style_manager.hpp"

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
    auto& editor_root = root.add<bgui::linear>(bgui::orientation::vertical);
    editor_root.style.layout.require_mode(
        bgui::mode::match_parent,
        bgui::mode::match_parent
    );
    auto& menu_bar = editor_root.add<bgui::menu_bar>(editor_root);
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
    SCOPED_TRACE(
        testing::Message()
        << "button content=" << action_button.get_content_size().x << "x"
        << action_button.get_content_size().y
        << ", label content=" << action_button.get_label().get_content_size().x
        << "x" << action_button.get_label().get_content_size().y
        << ", button width mode="
        << static_cast<int>(action_button.computed_style.layout.size_mode.x)
        << ", label width mode="
        << static_cast<int>(
            action_button.get_label().computed_style.layout.size_mode.x)
    );
    EXPECT_GE(
        action_button.get_content_size().x,
        action_button.get_label().get_content_size().x
    );
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
    EXPECT_GE(
        action_button.get_label().processed_x(),
        action_button.processed_x()
    );
    EXPECT_GE(
        action_button.get_label().processed_y(),
        action_button.processed_y()
    );
    EXPECT_LE(
        action_button.get_label().processed_x() +
            action_button.get_label().processed_width(),
        action_button.processed_x() + action_button.processed_width()
    );
    EXPECT_LE(
        action_button.get_label().processed_y() +
            action_button.get_label().processed_height(),
        action_button.processed_y() + action_button.processed_height()
    );
    click_at(center_of(action_button));
    EXPECT_EQ(callback_count, 1);

    click_at(center_of(menu_button));
    ASSERT_EQ(callback_count, 1);

    bgui::on_update();
    auto& popup_elements = editor_root.get_elements().at(bgui::layer::overlay);
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
    EXPECT_GE(option_button->get_label().processed_x(), option_button->processed_x());
    EXPECT_GE(option_button->get_label().processed_y(), option_button->processed_y());
    EXPECT_LE(
        option_button->get_label().processed_x() +
            option_button->get_label().processed_width(),
        option_button->processed_x() + option_button->processed_width()
    );
    EXPECT_LE(
        option_button->get_label().processed_y() +
            option_button->get_label().processed_height(),
        option_button->processed_y() + option_button->processed_height()
    );
    EXPECT_GE(
        option_button->processed_x(),
        popup->processed_x()
    );
    EXPECT_GE(
        option_button->processed_y(),
        popup->processed_y()
    );
    EXPECT_LE(
        option_button->processed_x() + option_button->processed_width(),
        popup->processed_x() + popup->processed_width()
    );
    EXPECT_LE(
        option_button->processed_y() + option_button->processed_height(),
        popup->processed_y() + popup->processed_height()
    );

    click_at(center_of(*option_button));
    EXPECT_EQ(callback_count, 2);
    EXPECT_FALSE(popup->is_enabled());
}

TEST(MenuBarTest, FileMenuSizesEachButtonAndPopupAroundDefaultFontText) {
    bgui::scoped_interface interface;
    bgui::style_manager::get_instance().apply_theme(bgui::dark_theme());
    auto& context = bgui::get_context();
    context.m_size = {800, 600};
    context.m_input_map.clear();
    context.m_last_mouse_left = false;

    auto& root = bgui::get_layout();
    auto& editor_root = root.add<bgui::linear>(bgui::orientation::vertical);
    editor_root.style.layout.require_mode(
        bgui::mode::match_parent,
        bgui::mode::match_parent
    );
    auto& menu_bar = editor_root.add<bgui::menu_bar>(editor_root);
    menu_bar.style.layout.require_mode(
        bgui::mode::match_parent,
        bgui::mode::wrap_content
    );
    auto& file_button = menu_bar.add_menu(
        "Files",
        0.3f,
        [](bgui::context_menu& menu) {
            menu.add_button("Save Project");
            menu.add_button("Load Project");
            menu.add_separator();
            menu.add_button("Save UI");
            menu.add_button("Load UI");
        }
    );

    bgui::on_update();
    click_at(center_of(file_button));
    bgui::on_update();

    auto& overlays = editor_root.get_elements().at(bgui::layer::overlay);
    ASSERT_EQ(overlays.size(), 1U);
    auto* popup = dynamic_cast<bgui::linear*>(overlays.front().get());
    ASSERT_NE(popup, nullptr);
    ASSERT_TRUE(popup->is_enabled());

    const auto& options = popup->get_elements().at(bgui::layer::base);
    ASSERT_EQ(options.size(), 5U);
    for (const auto& option : options) {
        auto* button = dynamic_cast<bgui::button*>(option.get());
        if (!button) {
            EXPECT_EQ(option->type, "element");
            EXPECT_GE(option->processed_x(), popup->processed_x());
            EXPECT_LE(
                option->processed_x() + option->processed_width(),
                popup->processed_x() + popup->processed_width()
            );
            continue;
        }
        auto& label = button->get_label();
        EXPECT_GE(
            button->get_content_size().y,
            label.get_content_size().y +
                button->computed_style.layout.padding.y +
                button->computed_style.layout.padding.w
        );
        EXPECT_GE(button->processed_width(), label.get_content_size().x);
        EXPECT_GE(button->processed_height(), label.get_content_size().y);
        EXPECT_GE(label.processed_x(), button->processed_x());
        EXPECT_GE(label.processed_y(), button->processed_y());
        EXPECT_LE(
            label.processed_x() + label.processed_width(),
            button->processed_x() + button->processed_width()
        );
        EXPECT_LE(
            label.processed_y() + label.processed_height(),
            button->processed_y() + button->processed_height()
        );
        EXPECT_GE(button->processed_x(), popup->processed_x());
        EXPECT_GE(button->processed_y(), popup->processed_y());
        EXPECT_LE(
            button->processed_x() + button->processed_width(),
            popup->processed_x() + popup->processed_width()
        );
        EXPECT_LE(
            button->processed_y() + button->processed_height(),
            popup->processed_y() + popup->processed_height()
        );
    }
}
