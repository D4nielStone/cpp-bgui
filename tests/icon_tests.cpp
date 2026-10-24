#include <bgui.hpp>
#include <gtest/gtest.h>

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
    const bgui::icon linux{"linux"};
    const bgui::icon windows{"windows"};

    EXPECT_EQ(linux.get_buffer(), "\xef\x85\xbc");
    EXPECT_EQ(windows.get_buffer(), "\xef\x85\xba");
}
