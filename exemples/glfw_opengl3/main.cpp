#include <bgui_backend_glfw.hpp>
#include <bgui_backend_opengl3.hpp>
#include <bgui.hpp>
#include <GLFW/glfw3.h>

#include <iostream>

int main() {

    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "BGUI Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // config layout
    bgui::init_lib();

    bgui_set_opengl3();
    bgui_set_glfw(window);

    auto& root = bgui::set_layout<blay::relative>(butil::orientation::horizontal); 
    // Supported layouts: linear, absolute (base), relative, and more.

    // Lateral panel: vertical linear layout
    auto& panel = root.add<blay::linear>(butil::orientation::vertical);

    // Cross alignment (horizontal)
    panel.set_cross_alignment(butil::alignment::stretch);
    panel.set_width(300/*, butil::pixel*/); // Pixel is default
    panel.set_height(1.f, butil::mode::relative);

    // Adding elements
    panel.add<belem::text>("Hello World!", 0.5f);

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();
        bgui_glfw_update_inputs();

        bgui::update();
        bgui_opengl3_render(
            bgui::get_draw_data()
        );

        glfwSwapBuffers(window);
    }

    // ============================
    // CLEANUP
    // ============================
    bgui::shutdown_lib();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}