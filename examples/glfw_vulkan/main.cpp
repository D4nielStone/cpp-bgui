#include <bgui.hpp>
#include <iostream>

int main() {
    // Setup
    GLFWwindow* window = bgui::set_up_glfw(1280, 720, "cpp-bgui Vulkan Example");
    bgui::set_up_vulkan(window);
    bgui::set_up_freetype();
    bgui::set_up();
    auto& sm = bgui::style_manager::get_instance();
    sm.apply_theme(bgui::dark_theme());
    bgui::set_font_antialiasing(true);

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
    auto& txt = panel.add<bgui::text>("Linear Layout Example", 0.4f);
    auto& button = panel.add<bgui::button>("Button Example", 0.4f, [](){});
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
        if(checked) {
            sm.apply_theme(bgui::dark_theme());
        } else {
            sm.apply_theme(bgui::light_theme());
        }
    });
    context.add<bgui::checkbox>("Allow the checkbox above", 0.4f, true)
        .set_on_change([&cb](bool checked){
            cb.set_enable(checked);
        });
    auto& txt2 = context.add<bgui::text>("FPS: ", 0.4f);
    auto& button2 = context.add<bgui::button>("Button inside window", 0.4f, [](){});
    auto& ia = context.add<bgui::input_area>("", 0.4f, [](const std::string& s){}, "Input area example");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        bgui::glfw_update(bgui::get_context());
        bgui::on_update();

        // Display fps in the title
        int fps = bgui::get_fps();
        txt2.set_buffer("FPS: " + std::to_string(fps));
        
        bgui::load_font_queue();
        bgui::vulkan_render(bgui::get_draw_data());
        glfwPollEvents();
    }

    // Cleanup
    bgui::shutdown_lib();
    bgui::shutdown_vulkan();
    bgui::shutdown_freetype();
    bgui::shutdown_glfw();
    return 0;
}