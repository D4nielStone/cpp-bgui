#include <bgui.hpp>
#include <iostream>

int main() {
    bgui::set_up();

    GLFWwindow* window = bkend::set_up_glfw(1280, 720, "BGUI Exemple");

    bkend::set_up_freetype();
    bkend::set_up_opengl3();

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
        bkend::glfw_update(); // update events
        bgui::update();       // update layout
        bkend::opengl3_render(
            bgui::get_draw_data() // render the layout data
        );
        glfwSwapBuffers(window);
    }

    bgui::shutdown_lib();
    bkend::shutdown_opengl3();
    bkend::shutdown_freetype();
    bkend::shutdown_glfw();
    return 0;

    return 0;
}