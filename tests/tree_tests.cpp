#include "bgui.hpp"

#include <gtest/gtest.h>

TEST(treeTest, ExpandsAndCollapsesChildren) {
    bgui::tree root("Root");
    auto& child = root.add_child("Child");
    auto& grandchild = child.add_child("Grandchild");

    ASSERT_EQ(root.type, "tree");
    EXPECT_FALSE(root.is_expanded());
    EXPECT_FALSE(root.children().is_enabled());
    EXPECT_EQ(child.type, "tree");
    EXPECT_FALSE(child.is_expanded());
    EXPECT_FALSE(child.children().is_enabled());
    EXPECT_EQ(grandchild.type, "tree");

    root.set_expanded(true);
    EXPECT_TRUE(root.is_expanded());
    EXPECT_TRUE(root.children().is_enabled());

    child.set_expanded(true);
    EXPECT_TRUE(child.children().is_enabled());

    root.set_expanded(false);
    EXPECT_FALSE(root.children().is_enabled());
    EXPECT_TRUE(child.is_expanded());
}

TEST(treeTest, CanStartExpanded) {
    bgui::tree root("Root", 0.4f, true);

    EXPECT_TRUE(root.is_expanded());
    EXPECT_TRUE(root.children().is_enabled());
}

TEST(treeTest, CanAddAndRemoveAnIcon) {
    bgui::tree root("Root");

    EXPECT_FALSE(root.has_icon());

    root.set_icon("directory_open");
    EXPECT_TRUE(root.has_icon());

    root.set_icon("");
    EXPECT_FALSE(root.has_icon());
}

TEST(treeTest, HasASeparateCompactDisclosureControl) {
    bgui::scoped_interface interface;
    bgui::tree root("Root");
    bgui::button* disclosure = nullptr;
    std::function<void(bgui::layout&)> visit = [&](bgui::layout& layout) {
        for (auto& [layer, children] : layout.get_elements())
            for (auto& child : children) {
                if (child->has_class("tree-disclosure"))
                    disclosure = dynamic_cast<bgui::button*>(child.get());
                if (auto* nested = child->as_layout())
                    visit(*nested);
            }
    };
    visit(root);
    ASSERT_NE(disclosure, nullptr);
    disclosure->on_pressed();
    disclosure->on_released();
    bgui::on_update();
    EXPECT_TRUE(root.is_expanded());
    disclosure->on_pressed();
    disclosure->on_released();
    bgui::on_update();
    EXPECT_FALSE(root.is_expanded());
}

TEST(treeTest, SelectionDisclosureLeavesAndDraggingAreIndependent) {
    bgui::scoped_interface interface;
    bgui::tree root("Root");
    int selections = 0;
    int drops = 0;
    root.header().set_function([&]() { ++selections; });
    root.set_on_drop([&](const bgui::vec2i&) { ++drops; });
    root.header().on_pressed();
    root.header().on_released();
    bgui::on_update();
    EXPECT_EQ(selections, 1);
    EXPECT_FALSE(root.is_expanded());
    root.disclosure().on_pressed();
    root.disclosure().on_released();
    bgui::on_update();
    EXPECT_EQ(selections, 1);
    EXPECT_TRUE(root.is_expanded());
    root.header().on_pressed();
    root.header().set_drag({8, 0});
    root.header().on_released();
    bgui::on_update();
    EXPECT_EQ(drops, 1);
    EXPECT_EQ(selections, 1);
    root.set_leaf(true);
    root.set_expanded(true);
    EXPECT_FALSE(root.is_expanded());
    EXPECT_FALSE(root.children().is_enabled());
}
