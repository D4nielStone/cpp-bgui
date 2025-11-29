#include <bgui.hpp>
#include <iostream>

int main() {
    GLFWwindow* window = bkend::set_up_glfw(800, 400, "BGUI GLFW & Vulkan Exemple");
    
    bkend::set_up_vulkan();
    bkend::set_up_freetype();

    bgui::set_up();

    auto* root = bgui::instance().get_layout();

    // Adding elements
    root->add<belem::text>("Hello World!", 2.f);

    while (!glfwWindowShouldClose(window)) {
        bkend::glfw_update();           // update events
        bgui::update();                 // update layout
        bkend::vulkan_render(
            bgui::get_draw_data()       // render the layout data
        );
        glfwSwapBuffers(window);
    }

    bgui::shutdown_lib();
    bkend::shutdown_vulkan();
    bkend::shutdown_freetype();
    bkend::shutdown_glfw();
    return 0;

    return 0;
}