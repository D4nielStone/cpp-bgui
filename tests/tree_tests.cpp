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
