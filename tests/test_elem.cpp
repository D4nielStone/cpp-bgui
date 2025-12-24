#include <gtest/gtest.h>
#include "elem/element.hpp"

using namespace bgui;
TEST(ElementTest, GetID) {
    element a, b, c, d("TagTest");

    // 1. Id tests (different elements may have different ids)
    EXPECT_EQ(a.get(), "#0");
    EXPECT_EQ(b.get(), "#1");
    EXPECT_EQ(c.get(), "#2");
    EXPECT_EQ(d.get(), "TagTest#3");
}

TEST(ElementTest, RequestSizeAndMode) {
    element elem;
    
    // 1. Reuqest Size Test (default mode is pixel)
    elem.request_size(150.f, 200.f);
    
    EXPECT_FLOAT_EQ(elem.requested_size()[0], 150.f);
    EXPECT_FLOAT_EQ(elem.requested_size()[1], 200.f);

    EXPECT_EQ(elem.get_requested_mode()[0], mode::pixel);
    EXPECT_EQ(elem.get_requested_mode()[1], mode::pixel);
    
    // 2. Test with percentual mode
    elem.request_width(mode::percent, 50.f);
    elem.request_height(mode::percent, 25.f);

    EXPECT_FLOAT_EQ(elem.requested_size()[0], 50.f);
    EXPECT_FLOAT_EQ(elem.requested_size()[1], 25.f);
    EXPECT_EQ(elem.get_requested_mode()[0], mode::percent);
    EXPECT_EQ(elem.get_requested_mode()[1], mode::percent);
}

TEST(ElementTest, FinalRectManipulation) {
    element elem;
    
    // 1. Manual final rect set
    elem.set_final_rect(10, 20, 100, 50);

    // 2. Verify getters
    EXPECT_EQ(elem.processed_x(), 10);
    EXPECT_EQ(elem.processed_y(), 20);
    EXPECT_EQ(elem.processed_width(), 100);
    EXPECT_EQ(elem.processed_height(), 50);

    EXPECT_EQ(elem.processed_position()[0], 10);
    EXPECT_EQ(elem.processed_size()[1], 50);
    EXPECT_EQ(elem.processed_rect()[3], 50); // height

    elem.set_position(5, 15);
    EXPECT_EQ(elem.processed_x(), 5);
    EXPECT_EQ(elem.processed_y(), 15);
    EXPECT_EQ(elem.processed_width(), 100);

    elem.set_final_size(80, 40);
    EXPECT_EQ(elem.processed_width(), 80);
    EXPECT_EQ(elem.processed_height(), 40);
    EXPECT_EQ(elem.processed_x(), 5);
}