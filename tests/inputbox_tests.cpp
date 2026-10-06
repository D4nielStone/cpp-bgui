#include <gtest/gtest.h>

#include "bgui.hpp"
#include "os/asset_manager.hpp"
#include "os/os.hpp"
#include "utils/syntax_highlight.hpp"

#include <regex>
#include <stdexcept>
#include <utility>

namespace {
    void ensure_test_font() {
        auto& fonts = bgui::font_manager::get_instance();
        if (fonts.has_font("default"))
            return;

        bgui::font test_font;
        test_font.ascent = 8.f;
        test_font.descent = 2.f;
        for (char32_t codepoint = 32; codepoint < 127; ++codepoint) {
            bgui::character glyph;
            glyph.size = {8, 10};
            glyph.bearing = {0, 8};
            glyph.advance = 8;
            test_font.chs.emplace(codepoint, glyph);
        }

        constexpr auto resolution = bgui::font_manager::m_default_resolution;
        bgui::asset_manager::get_instance().store_font(
            "input-area-test", resolution, std::move(test_font));
        fonts.set_default_font("input-area-test", resolution);
    }
}

TEST(InputAreaTest, CursorAndBackspaceKeepUtf8CodepointsIntact) {
    bgui::scoped_interface interface;
    auto& context = bgui::get_context();
    context.m_char_buffer.clear();
    context.m_input_map.clear();

    bgui::inputbox input("", "", 0.35f, [](const std::string&) {});
    input.set_focused(true);
    context.m_char_buffer = "\xC3\xA9";
    input.on_update();

    EXPECT_EQ(input.get_buffer(), "\xC3\xA9");
    EXPECT_EQ(input.get_cursor_position(), 2U);

    context.m_input_map[bgui::input_key::left] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 0U);

    context.m_input_map[bgui::input_key::left] = bgui::input_action::none;
    context.m_input_map[bgui::input_key::right] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 2U);

    context.m_input_map[bgui::input_key::right] = bgui::input_action::none;
    context.m_input_map[bgui::input_key::backspace] = bgui::input_action::press;
    input.on_update();
    EXPECT_TRUE(input.get_buffer().empty());
    EXPECT_EQ(input.get_cursor_position(), 0U);

    context.m_input_map.clear();
    context.m_char_buffer.clear();
}

TEST(InputAreaTest, NumberModeAcceptsOnlyNumericCharacters) {
    bgui::scoped_interface interface;
    auto& context = bgui::get_context();
    context.m_char_buffer.clear();
    context.m_input_map.clear();

    bgui::inputbox input("", "", 0.35f, nullptr, bgui::input_mode::number);
    input.set_focused(true);
    context.m_char_buffer = "a-12.3x.4";
    input.on_update();

    EXPECT_EQ(input.get_buffer(), "-12.34");
    EXPECT_EQ(input.get_cursor_position(), 6U);

    context.m_char_buffer.clear();
    context.m_input_map.clear();
}

TEST(InputAreaTest, CursorArrowPressMovesOneCharacterInsteadOfPerFrame) {
    bgui::scoped_interface interface;
    auto& context = bgui::get_context();
    context.m_char_buffer.clear();
    context.m_input_map.clear();

    bgui::inputbox input("abcd", "", 0.35f);
    input.set_focused(true);
    context.m_input_map[bgui::input_key::left] = bgui::input_action::press;

    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 3U);
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 3U);

    context.m_input_map[bgui::input_key::left] = bgui::input_action::release;
    input.on_update();
    context.m_input_map[bgui::input_key::left] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 2U);

    context.m_input_map.clear();
}

TEST(InputAreaTest, MultilineEnterAndVerticalArrowsMoveWithinAdjacentLines) {
    bgui::scoped_interface interface;
    auto& context = bgui::get_context();
    context.m_char_buffer.clear();
    context.m_input_map.clear();

    bgui::inputbox input("ab", "", 0.35f, nullptr, bgui::input_mode::multiline);
    input.set_focused(true);
    context.m_input_map[bgui::input_key::left] = bgui::input_action::press;
    input.on_update();
    context.m_input_map[bgui::input_key::left] = bgui::input_action::release;
    input.on_update();

    context.m_input_map[bgui::input_key::enter] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_buffer(), "a\nb");
    EXPECT_EQ(input.get_cursor_position(), 2U);
    input.on_update();
    EXPECT_EQ(input.get_buffer(), "a\nb");

    context.m_input_map[bgui::input_key::enter] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::left] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 1U);
    context.m_input_map[bgui::input_key::left] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::down] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 3U);

    context.m_input_map[bgui::input_key::down] = bgui::input_action::release;
    input.on_update();
    context.m_input_map[bgui::input_key::up] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 1U);

    context.m_input_map.clear();
}

TEST(InputAreaTest, DeleteRemovesOneFollowingUtf8Character) {
    bgui::scoped_interface interface;
    auto& context = bgui::get_context();
    context.m_char_buffer.clear();
    context.m_input_map.clear();

    bgui::inputbox input("\xC3\xA9x", "", 0.35f);
    input.set_focused(true);
    context.m_input_map[bgui::input_key::left] = bgui::input_action::press;
    input.on_update();
    ASSERT_EQ(input.get_cursor_position(), 2U);

    context.m_input_map[bgui::input_key::left] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::delete_key] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_buffer(), "\xC3\xA9");
    EXPECT_EQ(input.get_cursor_position(), 2U);

    context.m_input_map.clear();
}

TEST(InputAreaTest, MultilineHomeEndAndTabEditAtExpectedPositions) {
    bgui::scoped_interface interface;
    auto& context = bgui::get_context();
    context.m_char_buffer.clear();
    context.m_input_map.clear();

    bgui::inputbox input("first\nsecond", "", 0.35f, nullptr, bgui::input_mode::multiline);
    input.set_focused(true);

    context.m_input_map[bgui::input_key::home] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 6U);

    context.m_input_map[bgui::input_key::home] = bgui::input_action::release;
    input.on_update();
    context.m_input_map[bgui::input_key::end] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 12U);

    context.m_input_map[bgui::input_key::left_control] = bgui::input_action::press;
    context.m_input_map[bgui::input_key::home] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 0U);

    context.m_input_map[bgui::input_key::home] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::left_control] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::end] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::tab] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_buffer(), "\tfirst\nsecond");
    EXPECT_EQ(input.get_cursor_position(), 1U);
    input.on_update();
    EXPECT_EQ(input.get_buffer(), "\tfirst\nsecond");

    context.m_input_map.clear();
}

TEST(InputAreaTest, ShiftSelectsAndTypingReplacesTheSelectedText) {
    bgui::scoped_interface interface;
    auto& context = bgui::get_context();
    context.m_char_buffer.clear();
    context.m_input_map.clear();

    bgui::inputbox input("abcd", "", 0.35f, nullptr, bgui::input_mode::multiline);
    input.set_focused(true);
    context.m_input_map[bgui::input_key::left_shift] = bgui::input_action::press;
    context.m_input_map[bgui::input_key::left] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 3U);

    context.m_char_buffer = "X";
    input.on_update();
    EXPECT_EQ(input.get_buffer(), "abcX");
    EXPECT_EQ(input.get_cursor_position(), 4U);

    context.m_input_map.clear();
    context.m_char_buffer.clear();
}

TEST(InputAreaTest, MultilineControlCAndVUseClipboardCallbacks) {
    bgui::scoped_interface interface;
    auto& context = bgui::get_context();
    context.m_char_buffer.clear();
    context.m_input_map.clear();

    bgui::inputbox input("copy me", "", 0.35f, nullptr, bgui::input_mode::multiline);
    input.set_focused(true);
    std::string clipboard;
    context.m_set_clipboard = [&clipboard](const std::string& text) {
        clipboard = text;
    };
    context.m_get_clipboard = [&clipboard]() {
        return clipboard;
    };

    context.m_input_map[bgui::input_key::left_control] = bgui::input_action::press;
    context.m_input_map[bgui::input_key::a] = bgui::input_action::press;
    input.on_update();
    context.m_input_map[bgui::input_key::a] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::c] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(clipboard, "copy me");

    context.m_input_map[bgui::input_key::c] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::a] = bgui::input_action::press;
    input.on_update();
    context.m_input_map[bgui::input_key::a] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::x] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(clipboard, "copy me");
    EXPECT_TRUE(input.get_buffer().empty());

    context.m_input_map[bgui::input_key::x] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::v] = bgui::input_action::press;
    clipboard = "pasted";
    input.on_update();
    EXPECT_EQ(input.get_buffer(), "pasted");

    context.m_input_map.clear();
    context.m_get_clipboard = nullptr;
    context.m_set_clipboard = nullptr;
}

TEST(InputAreaTest, ControlArrowsAndDeleteOperateOnWholeWords) {
    bgui::scoped_interface interface;
    auto& context = bgui::get_context();
    context.m_char_buffer.clear();
    context.m_input_map.clear();

    bgui::inputbox input("one two", "", 0.35f, nullptr, bgui::input_mode::multiline);
    input.set_focused(true);

    context.m_input_map[bgui::input_key::home] = bgui::input_action::press;
    input.on_update();
    context.m_input_map[bgui::input_key::home] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::left_control] = bgui::input_action::press;

    context.m_input_map[bgui::input_key::right] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 4U);
    context.m_input_map[bgui::input_key::right] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::left] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_cursor_position(), 0U);

    context.m_input_map[bgui::input_key::left] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::delete_key] = bgui::input_action::press;
    input.on_update();
    EXPECT_EQ(input.get_buffer(), "two");
    EXPECT_EQ(input.get_cursor_position(), 0U);

    context.m_input_map[bgui::input_key::delete_key] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::end] = bgui::input_action::press;
    input.on_update();
    context.m_input_map[bgui::input_key::end] = bgui::input_action::release;
    context.m_input_map[bgui::input_key::backspace] = bgui::input_action::press;
    input.on_update();
    EXPECT_TRUE(input.get_buffer().empty());
    EXPECT_EQ(input.get_cursor_position(), 0U);

    context.m_input_map.clear();
}

TEST(InputAreaTest, MultilineWrapCanBeToggled) {
    bgui::scoped_interface interface;
    ensure_test_font();

    bgui::inputbox input(std::string(800, 'a'), "", 0.35f, nullptr, bgui::input_mode::multiline);
    EXPECT_TRUE(input.is_wrap_enabled());
    EXPECT_NE(input.get_label().get_vertical_cursor_position(0, 1), 0U);

    input.set_wrap(false);
    EXPECT_FALSE(input.is_wrap_enabled());
    EXPECT_EQ(input.get_label().get_vertical_cursor_position(0, 1), 0U);

    input.set_wrap(true);
    EXPECT_TRUE(input.is_wrap_enabled());
    EXPECT_NE(input.get_label().get_vertical_cursor_position(0, 1), 0U);
}

TEST(InputAreaTest, TextDrawCallsAreClippedToInputBoxRect) {
    bgui::scoped_interface interface;
    ensure_test_font();

    bgui::inputbox input("clipped", "");
    input.set_final_rect(20, 30, 40, 15);

    bgui::draw_data data;
    data.m_clip_rect = {0, 35, 100, 20};
    input.get_requires(&data);

    ASSERT_GT(data.m_quad_requires.size(), 1U);
    data.m_quad_requires.pop();
    while (!data.m_quad_requires.empty()) {
        EXPECT_EQ(data.m_quad_requires.front().m_clip_rect, (bgui::vec4i{20, 35, 40, 10}));
        data.m_quad_requires.pop();
    }
}

TEST(InputAreaTest, LoadsOrderedRegexHighlightRulesFromJsonAsset) {
    const auto rules = bgui::load_syntax_highlight_config("lua_highlight.json");

    ASSERT_EQ(rules.size(), 6U);
    EXPECT_TRUE(std::regex_search(std::string("\"text\""), rules[0].expression));
    EXPECT_TRUE(std::regex_search(std::string("-- function name"), rules[1].expression));
    EXPECT_TRUE(std::regex_search(std::string("function"), rules[2].expression));
    EXPECT_TRUE(std::regex_search(std::string("return"), rules[2].expression));
    EXPECT_TRUE(std::regex_search(std::string("function my_func"), rules[3].expression));
    EXPECT_TRUE(std::regex_search(std::string("{"), rules[4].expression));
    EXPECT_TRUE(std::regex_search(std::string("}"), rules[4].expression));
    EXPECT_FLOAT_EQ(rules[2].color.r, 0x56 / 255.f);
    EXPECT_FLOAT_EQ(rules[2].color.g, 0x9C / 255.f);
    EXPECT_FLOAT_EQ(rules[3].color.r, 0xDC / 255.f);
    EXPECT_FLOAT_EQ(rules[4].color.r, 0xDC / 255.f);
}

TEST(InputAreaTest, MissingHighlightConfigurationReportsAnError) {
    EXPECT_THROW(
        bgui::load_syntax_highlight_config("missing_highlight_config.json"),
        std::runtime_error);
}
