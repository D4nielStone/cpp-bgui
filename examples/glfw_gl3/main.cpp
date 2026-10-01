#include <bgui.hpp>
#include <iostream>

int main() {
    // Setup
    bgui::set_up();
    auto& sm = bgui::style_manager::get_instance();
    sm.apply_theme(bgui::dark_theme());
    
    GLFWwindow* window = bgui::set_up_glfw(1280, 720, "BGUI GLFW & gl3 Example");
    bgui::set_up_gl3();
    bgui::set_up_freetype();
    bgui::set_global_scale(0.9f);
    
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
    panel.add<bgui::text>("Linear Layout Example\nYou can add more widgets here.", 0.4f);
    panel.add<bgui::image>("assets/bubble.png", [](const std::string& path){
        std::cout << "Image clicked: " << path << std::endl;
    })
        .set_size_mode(bgui::mode::match_parent, bgui::mode::same);
    panel.add<bgui::button>("Button Example", 0.4f, [](){});
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
    context.add<bgui::text>("This is a window widget example.", 0.4f);
    auto& cb = context.add<bgui::checkbox>("Switch theme", 0.4f, false);
    cb.set_on_change([&sm](bool checked){
        if(!checked) {
            sm.apply_theme(bgui::dark_theme());
        } else {
            sm.apply_theme(bgui::light_theme());
        }
    });
    context.add<bgui::checkbox>("Allow the checkbox above", 0.4f, true)
        .set_on_change([&cb](bool checked){
            cb.set_enable(checked);
        });
    context.add<bgui::button>("Button inside window", 0.4f, [](){});
    context.add<bgui::input_area>("", 0.4f, [](const std::string& s){}, "Input area example");

    bgui::get_context().m_refresh_func = [&](){
        bgui::glfw_update(bgui::get_context());
        bgui::load_font_queue();
        bgui::on_update();
        bgui::gl3_clear();
        bgui::gl3_render(bgui::get_draw_data());
        glfwSwapBuffers(window);
    };

    bgui::glfw_main_loop();

    // Cleanup
    bgui::shutdown_lib();
    bgui::shutdown_gl3();
    bgui::shutdown_freetype();
    bgui::shutdown_glfw();
    return 0;
}
