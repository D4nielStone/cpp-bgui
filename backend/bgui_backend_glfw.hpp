#pragma once
#include <GLFW/glfw3.h>

namespace bkend {
    GLFWwindow* set_up_glfw(int width, int height, const char* title, int flags = 0, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);
    void glfw_update();
    void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    void shutdown_glfw();
} // namespace bkend