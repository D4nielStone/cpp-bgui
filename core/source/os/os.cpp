#include <fstream>
#include <sstream>
#include <iostream>
#include "os/os.hpp"

butil::vec2i bos::get_window_size() {
    /*
    // verify if glfw was iniitalized
    if (glfwInit() == GLFW_TRUE) {
        GLFWwindow* window = glfwGetCurrentContext();
        if (window) {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            return butil::vec2i{width, height};
        }
    }*/ // TODO: implement on backend
    return butil::vec2i{800, 600};
}
butil::vec2i bos::get_mouse_position() {
    /*
    if (glfwInit() == GLFW_TRUE) {
        GLFWwindow* window = glfwGetCurrentContext();
        if (window) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            return butil::vec2i{(int)x, (int)y};
        }
    }*/ //TODO: implement on backend
    return butil::vec2i{0, 0};
}
std::string bos::read_file(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path + "\n");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

butil::mat4 bos::get_projection() {
    auto size = bos::get_window_size();
    auto width = size[0];
    auto height = size[1];
    return butil::orthographic(0.0f, static_cast<float>(width), static_cast<float>(height), 0.f);
}

bool bos::get_pressed(const bos::input_key& k) {
    /*
    // verify callback
    static bool has_set_callback = false;
    if(!has_set_callback) {
        // verify if glfw was iniitalized
        if (glfwInit() == GLFW_TRUE) {
            GLFWwindow* window = glfwGetCurrentContext();
            if (window) {
                glfwSetMouseButtonCallback(window, bos::glfw_mouse_button_callback);
                has_set_callback = true;
            }
        }
    }
    *///TODO: implement on backend
    if(bos::s_input_map[k] == bos::input_action::press) return true;
    return false;
}
/*
void bos::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    bos::input_key internal_key = bos::s_glfw_key_reverse_map.at(button);

    bos::input_action internal_action = static_cast<bos::input_action>(bos::s_glfw_action_reverse_map.at(action)); 

    bos::s_input_map[internal_key] = internal_action;
}*/// TODO: move to backend