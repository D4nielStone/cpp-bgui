#include <bgui.hpp>
#include <iostream>

int main() {
    // Setup
    GLFWwindow* window = bgui::set_up_glfw(1280, 720, "cpp-bgui Vulkan Example");
    bgui::set_up_vulkan(window);
    bgui::set_up_freetype();
    {
        bgui::scoped_interface interface_scope;
        auto& sm = bgui::style_manager::get_instance();
        sm.apply_theme(bgui::dark_theme());
        bgui::set_font_antialiasing(true);

        // Keep each handle alive while its element should remain in the layout.
        bgui::layout& root = bgui::get_layout();

        auto panel = root.add<bgui::linear>(bgui::orientation::vertical);
        panel->style = {
            .layout = {
                .size_mode = std::make_optional<bgui::vec<2UL, bgui::mode>>({bgui::mode::pixel, bgui::mode::match_parent}),
                .size = std::make_optional<bgui::vec<2UL, float>>({300.f, 1.f})
            },
            .visual = {
                .visible = true
            }
        };
        auto panel_text = panel->add<bgui::text>("Linear Layout Example", 0.4f);
        auto panel_button = panel->add<bgui::button>("Button Example", 0.4f, [](){});
        auto demo_window = root.add<bgui::window>("Hello Bubble!");

        auto context = demo_window->add<bgui::linear>(bgui::orientation::vertical);
        context->style = {
            .layout = {
                .size_mode = std::make_optional<bgui::vec<2UL, bgui::mode>>({bgui::mode::match_parent, bgui::mode::match_parent})
            },
            .visual = {
                .visible = false
            }
        };
        auto window_text = context->add<bgui::text>("This is a window widget example.", 0.4f);
        auto theme_checkbox = context->add<bgui::checkbox>("Switch theme", 0.4f, false);
        theme_checkbox->set_on_change([&sm](bool checked){
            if(checked) {
                sm.apply_theme(bgui::dark_theme());
            } else {
                sm.apply_theme(bgui::light_theme());
            }
        });
        auto enable_checkbox = context->add<bgui::checkbox>("Allow the checkbox above", 0.4f, true);
        enable_checkbox->set_on_change([&theme_checkbox](bool checked){
            theme_checkbox->set_enable(checked);
            });
        auto fps_text = context->add<bgui::text>("FPS: ", 0.4f);
        auto window_button = context->add<bgui::button>("Button inside window", 0.4f, [](){});
        auto inputbox = context->add<bgui::inputbox>("", 0.4f, [](const std::string& s){}, "Input box example");

        // Main loop
        while (!glfwWindowShouldClose(window)) {
            bgui::glfw_update(bgui::get_context());
            bgui::on_update();

            // Display fps in the title
            int fps = bgui::get_fps();
            fps_text->set_buffer("FPS: " + std::to_string(fps));

            bgui::load_font_queue();
            bgui::vulkan_render(bgui::get_draw_data());
            glfwPollEvents();
        }
    }

    // Cleanup
    bgui::shutdown_vulkan();
    bgui::shutdown_freetype();
    bgui::shutdown_glfw();
    return 0;
}