#include <gtest/gtest.h>

#include "bgui.hpp"
#include "os/os.hpp"

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
