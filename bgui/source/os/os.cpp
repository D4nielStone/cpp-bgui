#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>
#include "os/os.hpp"

bgui_utils::vec2i bgui_os::get_window_size() {
    if (glfwInit() == GLFW_TRUE) {
        GLFWwindow* window = glfwGetCurrentContext();
        if (window) {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            return bgui_utils::vec2i{width, height};
        }
    }
    return bgui_utils::vec2i{800, 600};
}
std::string bgui_os::read_file(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bgui_utils::mat4 bgui_os::get_projection() {
    bgui_utils::mat4 proj;
    auto size = bgui_os::get_window_size();
    auto width = size[0];
    auto height = size[1];
    bgui_utils::orthographic(proj, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f);
    return proj;
}