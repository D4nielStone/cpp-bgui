#include <gtest/gtest.h>
#include <glad/glad.h>
#include <bgui.hpp>
#include <GLFW/glfw3.h>
#include <array>

namespace {
class GL3Test : public testing::Test {
protected:
    GLFWwindow* window{};
    bgui::vec2i previous_size;

    void SetUp() override {
        if (!glfwInit())
            GTEST_SKIP() << "No GLFW display available";
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(64, 64, "BGUI rendering test", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            GTEST_SKIP() << "OpenGL 3.3 context unavailable";
        }
        glfwMakeContextCurrent(window);
        bgui::set_up_gl3();
        previous_size = bgui::get_context().m_size;
        bgui::get_context().m_size = {64, 64};
        glViewport(0, 0, 64, 64);
        glDisable(GL_FRAMEBUFFER_SRGB);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void TearDown() override {
        if (!window)
            return;
        bgui::get_context().m_size = previous_size;
        bgui::shutdown_gl3();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    std::array<unsigned char, 4> pixel(int x, int y) {
        std::array<unsigned char, 4> result{};
        glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, result.data());
        return result;
    }
};
}

TEST_F(GL3Test, ImageRoundsCornersAndPreservesTextureAndBorder) {
    bgui::texture texture;
    texture.m_path = "rounded-image-test";
    texture.m_size = {1.f, 1.f};
    texture.m_has_alpha = true;
    texture.m_generate_mipmap = false;
    texture.m_buffer = {255, 0, 0, 255};
    bgui::image image(texture);
    image.style.visual.border_radius = 12.f;
    image.style.visual.border_size = 2.f;
    image.style.visual.border.normal = bgui::color{0.f, 1.f, 0.f, 1.f};
    image.compute_style();
    image.set_final_rect(8, 8, 48, 48);
    bgui::draw_data data;
    data.m_clip_rect = {0, 0, 64, 64};
    image.get_requires(&data);
    bgui::gl3_render(&data);

    EXPECT_EQ(pixel(8, 8)[0], 0); // Rounded corner exposes the background.
    EXPECT_EQ(pixel(55, 55)[0], 0);
    EXPECT_EQ(pixel(32, 32)[0], 255); // Texture remains visible in the center.
    EXPECT_EQ(pixel(32, 8)[1], 255); // Border follows the image contour.
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}
