#include <bgui.hpp>
#include <iostream>

int main() {
    GLFWwindow* window = bgui::set_up_glfw(1280, 720, "BGUI GLFW & gl3 Exemple");
    
    bgui::set_up_gl3();
    bgui::set_up_freetype();

    bgui::set_up();
    bgui::layout& root = bgui::get_layout();

    // Adding elements
    auto& panel = root.add<bgui::linear>(bgui::orientation::vertical);
    panel.set_padding(10, 2);
    panel.request_width(bgui::mode::pixel, 300.f);
    panel.request_height(bgui::mode::match_parent);

    // layout are invisible by default
    panel.set_visible(true);

    auto& txt = panel.add<bgui::text>("Linear Layout Exemple", 0.5f);
    txt.request_width(bgui::mode::match_parent);
    txt.set_alignment(bgui::alignment::center);
    auto& button = panel.add<bgui::button>("Button Exemple", 0.5f, [](){});
    button.request_width(bgui::mode::match_parent);

    // window widget
    auto& win = root.add<bgui::window>("Hello Bubble!");
    auto& context = win.add<bgui::linear>(bgui::orientation::vertical);
    context.request_height(bgui::mode::stretch);
    context.request_width(bgui::mode::match_parent);
    context.set_padding(10, 10);

    context.add<bgui::text>("This is a window widget exemple.", 0.5f);
    context.add<bgui::checkbox>(nullptr, false, "Checkbox Exemple", 0.5f);
    auto& txt2 = context.add<bgui::text>("Centered text", 0.5f);
    txt2.set_alignment(bgui::alignment::center);
    txt2.request_width(bgui::mode::stretch);
    auto& button2 = context.add<bgui::button>("Button inside window", 0.5f, [](){});
    button2.request_width(bgui::mode::match_parent);

    // style must be applyed in the end
    bgui::apply_style(bgui::dark_style);

    while (!glfwWindowShouldClose(window)) {
        bgui::glfw_update(bgui::get_context());           // update events
        bgui::on_update();                 // update layout
        bgui::gl3_render(
            bgui::get_draw_data()       // render the layout data
        );
        glfwSwapBuffers(window);
    }

    bgui::shutdown_lib();
    bgui::shutdown_gl3();
    bgui::shutdown_freetype();
    bgui::shutdown_glfw();
    return 0;
}