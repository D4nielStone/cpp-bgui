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

TEST(ComboBoxTest, DropdownUsesTheRootOverlayLayer) {
    bgui::scoped_interface interface;
    auto& root = bgui::get_layout();
    root.set_final_rect(0, 0, 800, 600);
    auto& control = root.add_persistent<bgui::combo_box>(
        std::vector<std::string>{"First", "Second"}
    );
    control.set_final_rect(10, 20, 120, 30);
    root.cascade_style();

    control.on_clicked();
    root.on_update();

    const auto& root_overlay = root.get_elements().at(bgui::layer::overlay);
    ASSERT_EQ(root_overlay.size(), 1U);
    EXPECT_EQ(root_overlay.front()->get_parent(), &root);
    EXPECT_EQ(root_overlay.front()->processed_x(), 10);
    EXPECT_EQ(root_overlay.front()->processed_y(), 50);
    EXPECT_EQ(root_overlay.front()->processed_width(), 120);
    EXPECT_TRUE(root_overlay.front()->is_enabled());
    EXPECT_TRUE(control.get_elements().find(bgui::layer::overlay) ==
        control.get_elements().end());
}
