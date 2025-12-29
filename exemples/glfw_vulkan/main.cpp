#include <bgui.hpp>
#include <iostream>

int main() {
    GLFWwindow* window = bgui::set_up_glfw(800, 400, "BGUI GLFW & Vulkan Exemple");
    
    bgui::set_up_vulkan();
    bgui::set_up_freetype();

    bgui::set_up();

    auto* root = bgui::get_layout();

    // Adding elements
    root->add<bgui::text>("Hello World!", 2.f);

    while (!glfwWindowShouldClose(window)) {
        bgui::glfw_on_update();           // update events
        bgui::on_update();                 // update layout
        bgui::vulkan_render(
            bgui::get_draw_data()       // render the layout data
        );
        glfwSwapBuffers(window);
    }

    bgui::shutdown_lib();
    bgui::shutdown_vulkan();
    bgui::shutdown_freetype();
    bgui::shutdown_glfw();
    return 0;

    return 0;
}