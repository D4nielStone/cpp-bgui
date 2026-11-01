#include <gtest/gtest.h>

// Windows headers can define these legacy qualifiers before BGUI is included.
#define near
#define far
#include "utils/mat.hpp"
#undef near
#undef far

TEST(AuditTest, OrthographicMatrixAcceptsWindowsQualifierMacros) {
    const auto matrix = bgui::orthographic(-2.f, 2.f, -4.f, 4.f, .1f, 10.f);
    EXPECT_FLOAT_EQ(matrix(0, 0), .5f);
    EXPECT_FLOAT_EQ(matrix(1, 1), .25f);
    EXPECT_NEAR(matrix(2, 2), -2.f / 9.9f, .000001f);
    EXPECT_NEAR(matrix(3, 2), -10.1f / 9.9f, .000001f);
}
