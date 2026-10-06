#include <gtest/gtest.h>
#include "bgui.hpp"

#include <stdexcept>

TEST(ComboBoxTest, InitializesSelectionAndNotifiesOnlyWhenSelectionChanges) {
    bgui::combo_box control({"Apple", "Orange", "Pear"}, 0.4f, 1);
    EXPECT_EQ(control.get_selected_index(), 1U);
    EXPECT_EQ(control.get_selected_option(), "Orange");

    int callback_count = 0;
    std::size_t callback_index = 0;
    std::string callback_option;
    control.set_on_change(
        [&](std::size_t index, const std::string& option) {
            ++callback_count;
            callback_index = index;
            callback_option = option;
        }
    );

    control.set_selected_index(2);
    EXPECT_EQ(control.get_selected_index(), 2U);
    EXPECT_EQ(control.get_selected_option(), "Pear");
    EXPECT_EQ(callback_count, 1);
    EXPECT_EQ(callback_index, 2U);
    EXPECT_EQ(callback_option, "Pear");

    control.set_selected_index(2);
    EXPECT_EQ(callback_count, 1);
}

TEST(ComboBoxTest, RejectsEmptyOptionsAndInvalidSelectionIndices) {
    EXPECT_THROW(bgui::combo_box({}), std::invalid_argument);
    EXPECT_THROW(
        bgui::combo_box({"First", "Second"}, 0.4f, 2),
        std::out_of_range
    );

    bgui::combo_box control({"First"});
    EXPECT_THROW(control.set_selected_index(1), std::out_of_range);
}