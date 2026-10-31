#define near
#define far
#include "utils/mat.hpp"
#undef far
#undef near
#include <gtest/gtest.h>

TEST(MatrixCompatibility, OrthographicAcceptsWindowsNearFarMacros) {
    const auto matrix = bgui::orthographic(-1.f, 1.f, -1.f, 1.f, 2.f, 10.f);
    EXPECT_FLOAT_EQ(matrix(0, 0), 1.f);
    EXPECT_FLOAT_EQ(matrix(1, 1), 1.f);
    EXPECT_FLOAT_EQ(matrix(2, 2), -0.25f);
    EXPECT_FLOAT_EQ(matrix(3, 2), -1.5f);
}
