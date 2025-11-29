#include <bgui_backend_glfw.hpp>
#include <os/os.hpp>
#include <stdexcept>
#include <iostream>

static std::unordered_map<int, bos::input_key> s_glfw_key_reverse_map = {
    {GLFW_MOUSE_BUTTON_LEFT, bos::input_key::mouse_left},
    {GLFW_MOUSE_BUTTON_RIGHT, bos::input_key::mouse_right},
    {GLFW_MOUSE_BUTTON_MIDDLE, bos::input_key::mouse_middle}
};
static std::unordered_map<int, bos::input_action> s_glfw_action_reverse_map = {
    {GLFW_PRESS, bos::input_action::press},
    {GLFW_RELEASE, bos::input_action::release},
    {GLFW_REPEAT, bos::input_action::repeat}
};

GLFWwindow* bkend::set_up_glfw(int width, int height, const char* title, int flags, GLFWmonitor* monitor, GLFWwindow* share) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to init GLFW\n");
    }
    #ifdef BGUI_USE_OPENGL
        std::cout << "Using OpenGL backend to GLFW\n";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #endif
    GLFWwindow* window = glfwCreateWindow(width, height, title, monitor, share);

    if (!window) {
        throw std::runtime_error("Failed to create window\n");
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync
    return window;
}
void bkend::glfw_update() {
    glfwPollEvents();
}

void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    bos::input_key internal_key = s_glfw_key_reverse_map.at(button);

    bos::input_action internal_action = static_cast<bos::input_action>(s_glfw_action_reverse_map.at(action)); 

    bos::s_input_map[internal_key] = internal_action;
}
void bkend::shutdown_glfw() {
    glfwDestroyWindow(glfwGetCurrentContext());
    glfwTerminate();
}