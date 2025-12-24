#include <gtest/gtest.h>
#include "elem/element.hpp"

TEST(ElementTest, Id) {
    bgui::element a, b, c;
    EXPECT_EQ(a.get_id(), 0);
    EXPECT_EQ(b.get_id(), 1);
    EXPECT_EQ(c.get_id(), 2);
}