#include <bgui.hpp>
#include <iostream>

int main() {
    // Setup
    bgui::set_up();
    auto& sm = bgui::style_manager::get_instance();
    sm.apply_theme(bgui::dark_theme());
    
    GLFWwindow* window = bgui::set_up_glfw(1000, 500, "BGUI GLFW & gl3 Exemple");
    bgui::set_up_gl3();
    bgui::set_up_freetype();
    
    // Build UI declaratively
    bgui::layout& root = bgui::get_layout();

    auto& panel = root.add<bgui::linear>(bgui::orientation::vertical);
    panel.style = {
        .layout = {
            .size_mode = std::make_optional<bgui::vec<2UL, bgui::mode>>({bgui::mode::pixel, bgui::mode::match_parent}),
            .size = std::make_optional<bgui::vec<2UL, float>>({300.f, 1.f})
        },
        .visual = {
            .visible = true
        }
    };
    auto& txt = panel.add<bgui::text>("Linear Layout Exemple", 0.35f);
    auto& button = panel.add<bgui::button>("Button Exemple", 0.35f, [](){});
    auto& win = root.add<bgui::window>("Hello Bubble!");
    
    auto& context = win.add<bgui::linear>(bgui::orientation::vertical);
    context.style = {
        .layout = {
            .size_mode = std::make_optional<bgui::vec<2UL, bgui::mode>>({bgui::mode::match_parent, bgui::mode::match_parent})
        },
        .visual = {
            .visible = false
        }
    };
    context.add<bgui::text>("This is a window widget exemple.", 0.35f);
    auto& cb = context.add<bgui::checkbox>("Switch theme", 0.35f, false);
    cb.set_on_change([&sm](bool checked){
        if(checked) {
            sm.apply_theme(bgui::dark_theme());
        } else {
            sm.apply_theme(bgui::light_theme());
        }
    });
    context.add<bgui::checkbox>("Allow the checkbox above", 0.35f, true)
        .set_on_change([&cb](bool checked){
            cb.set_enable(checked);
        });
    auto& txt2 = context.add<bgui::text>("FPS: ", 0.35f);
    auto& button2 = context.add<bgui::button>("Button inside window", 0.35f, [](){});
    //auto& ia = context.add<bgui::input_area>("", 0.35f, "Input area exemple");

    bgui::get_context().m_refresh_func = [&](){
        bgui::on_update();

        // Display fps in the title
        int fps = bgui::get_fps();
        txt2.set_buffer("FPS: " + std::to_string(fps));
        
        bgui::load_font_queue();
        bgui::gl3_clear();
        bgui::gl3_render(bgui::get_draw_data());
        glfwSwapBuffers(window);
    };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // Cleanup
    bgui::shutdown_lib();
    bgui::shutdown_gl3();
    bgui::shutdown_freetype();
    bgui::shutdown_glfw();
    return 0;
}
