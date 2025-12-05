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

    auto& txt = panel.add<bgui::text>("Linear Layout Exemple", 0.35f);
    txt.request_width(bgui::mode::match_parent);
    txt.set_alignment(bgui::alignment::center);
    auto& button = panel.add<bgui::button>("Button Exemple", 0.35f, [](){});
    button.request_width(bgui::mode::match_parent);

    // window widget experiment
    auto& win = root.add<bgui::linear>(bgui::orientation::vertical);
    win.set_final_position(200, 100);
    win.request_size(400, 300);
    win.get_material().set("bg_color", bgui::color({0.3f, 0.3f, 0.3f, 1.f}));
    win.set_visible(true);

    // testing the header:
    auto& header = win.add<bgui::linear>(bgui::orientation::horizontal);
    header.get_material().set("bg_color", bgui::color({0.2f, 0.2f, 0.2f, 1.f}));
    header.request_height(bgui::mode::wrap_content);
    header.request_width(bgui::mode::match_parent);
    header.add<bgui::button>("[icon]", 0.35f, [](){});
    header.add<bgui::text>("Window Title", 0.35f).request_width(bgui::mode::stretch);
    header.add<bgui::button>("[X]", 0.35f, [](){});
    header.set_visible(true);

    // Theme must be applyed in the end
    bgui::apply_theme(bgui::dark_theme);

    while (!glfwWindowShouldClose(window)) {
        bgui::glfw_update(bgui::get_window());           // update events
        bgui::update();                 // update layout
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

    return 0;
}