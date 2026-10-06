#include "bgui.hpp"

#include <gtest/gtest.h>

TEST(VectorFieldTest, CreatesNumericInputForEachAxis) {
    bgui::vector_field field("Position", {"x:", "y:", "z:"});

    ASSERT_EQ(field.type, "vector_field");
    ASSERT_EQ(field.get_component_count(), 3u);
    EXPECT_EQ(field.get_component(0).get_input_mode(), bgui::input_mode::number);
    EXPECT_EQ(field.get_component(1).get_input_mode(), bgui::input_mode::number);
    EXPECT_EQ(field.get_component(2).get_input_mode(), bgui::input_mode::number);
    for (std::size_t index = 0; index < field.get_component_count(); ++index) {
        const auto& input = field.get_component(index);
        ASSERT_TRUE(input.style.layout.size_mode.has_value());
        EXPECT_EQ(input.style.layout.size_mode.value()[1], bgui::mode::pixel);
        ASSERT_TRUE(input.style.layout.size.has_value());
        EXPECT_FLOAT_EQ(input.style.layout.size.value()[1], 20.f);
    }
}
