#include <gtest/gtest.h>
#include "bgui.hpp"

using namespace bgui;

TEST(ImageTest, UsesNaturalTextureSizeForWrapContent) {
    texture source;
    source.m_size = {320.f, 180.f};
    source.m_buffer = {255, 255, 255, 255};

    image image_element(source);
    image_element.compute_style();
    image_element.calc_content_size(layer::base);
    image_element.process_required_size({800, 600});

    EXPECT_EQ(image_element.processed_width(), 320);
    EXPECT_EQ(image_element.processed_height(), 180);
}

TEST(ImageTest, SupportsExplicitSize) {
    texture source;
    source.m_size = {320.f, 180.f};
    source.m_buffer = {255, 255, 255, 255};

    image image_element(source);
    image_element.set_size(640.f, 360.f);
    image_element.compute_style();
    image_element.calc_content_size(layer::base);
    image_element.process_required_size({800, 600});

    EXPECT_EQ(image_element.processed_width(), 640);
    EXPECT_EQ(image_element.processed_height(), 360);
}

TEST(ImageTest, SupportsParentMatchingMode) {
    texture source;
    source.m_size = {64.f, 64.f};
    source.m_buffer = {255, 255, 255, 255};

    image image_element(source);
    image_element.set_size_mode(mode::match_parent, mode::match_parent);
    image_element.compute_style();
    image_element.calc_content_size(layer::base);
    image_element.process_required_size({500, 300});

    EXPECT_EQ(image_element.processed_width(), 500);
    EXPECT_EQ(image_element.processed_height(), 300);
}
