#include <gtest/gtest.h>
#include "bgui.hpp"

#include <stdexcept>

TEST(ComboBoxTest, InitializesSelectionAndNotifiesOnlyWhenSelectionChanges) {
    bgui::combo_box control({"Apple", "Orange", "Pear"}, 0.3, 1);
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
        bgui::combo_box({"First", "Second"}, 0.3, 2),
        std::out_of_range
    );

    bgui::combo_box control({"First"});
    EXPECT_THROW(control.set_selected_index(1), std::out_of_range);
}

TEST(ContextMenuTest, ComputesPopupAndItemStylesWhenFirstOpened) {
    bgui::scoped_interface interface;
    auto& root = bgui::get_layout();
    auto& anchor = root.add<bgui::button>("Menu", 0.3f, []() {});
    bgui::context_menu menu(root, anchor);
    menu.add_item("A long menu item", 0.3f, []() {});

    root.cascade_style();
    menu.toggle();

    auto& popup = *root.get_elements()[bgui::layer::overlay].back();
    ASSERT_NE(popup.as_layout(), nullptr);
    EXPECT_EQ(popup.computed_style.layout.limit_min.x, 200);

    auto& item = *popup.as_layout()->get_elements()[bgui::layer::base].front();
    EXPECT_EQ(
        item.computed_style.layout.size_mode.x,
        bgui::mode::match_parent
    );
}
TEST(ComboBoxTest, QuickOutsidePressClosesPopupAtTheEventPosition) {
    bgui::scoped_interface interface;
    auto& root = bgui::get_layout();
    auto control = root.add_scoped<bgui::combo_box>(std::vector<std::string>{"First"});
    control->style.layout.require_size(100, 30);
    control->on_clicked();
    auto& popup = *root.get_elements().at(bgui::layer::overlay).back();
    ASSERT_TRUE(popup.is_enabled());

    auto& io = bgui::get_context();
    io.m_input_map[bgui::input_key::mouse_left] = bgui::input_action::release;
    io.m_mouse_left_events.push_back({bgui::input_action::press, {400, 300}});
    io.m_mouse_left_events.push_back({bgui::input_action::release, {400, 300}});
    io.m_mouse_position = {0, 0};
    bgui::on_update();
    EXPECT_FALSE(popup.is_enabled());
}

TEST(ComboBoxTest, DestroysOpenPopupWithRootLayout) {
    bgui::scoped_interface interface;
    auto& root = bgui::get_layout();
    auto& panel = root.add<bgui::linear>(bgui::orientation::vertical);
    auto& control = panel.add<bgui::combo_box>(std::vector<std::string>{"First"});

    control.on_clicked();
    ASSERT_TRUE(root.get_elements().at(bgui::layer::overlay).back()->is_enabled());
}

TEST(ComboBoxTest, RemovesModalAndItsPopupMenusFromSameOverlay) {
    bgui::scoped_interface interface;
    auto& root = bgui::get_layout();
    auto& modal = root.add<bgui::modal, bgui::layer::overlay>();
    auto& first = modal.add<bgui::combo_box>(std::vector<std::string>{"All", "Physics"});
    auto& second = modal.add<bgui::combo_box>(std::vector<std::string>{"All", "Methods"});
    first.on_clicked();
    second.on_clicked();
    ASSERT_EQ(root.get_elements().at(bgui::layer::overlay).size(), 3u);

    EXPECT_TRUE(root.remove(&modal));
    EXPECT_TRUE(root.get_elements().at(bgui::layer::overlay).empty());
}

TEST(ComboBoxTest, RemovesModalWithOnePopupAndPreservesFollowingOverlay) {
    bgui::scoped_interface interface;
    auto& root = bgui::get_layout();
    auto& modal = root.add<bgui::modal, bgui::layer::overlay>();
    auto& control = modal.add<bgui::combo_box>(std::vector<std::string>{"All"});
    control.on_clicked();
    auto& following = root.add<bgui::linear, bgui::layer::overlay>(bgui::orientation::vertical);
    following.add<bgui::text>("Keep this overlay", .3f);
    ASSERT_EQ(root.get_elements().at(bgui::layer::overlay).size(), 3u);
    EXPECT_TRUE(root.remove(&modal));
    ASSERT_EQ(root.get_elements().at(bgui::layer::overlay).size(), 1u);
    EXPECT_EQ(root.get_elements().at(bgui::layer::overlay).front().get(), &following);
}
