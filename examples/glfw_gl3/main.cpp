#include <bgui.hpp>
#include <iostream>

int main() {
    // Setup
    GLFWwindow* window = bgui::set_up_glfw(1280, 720, "BGUI GLFW & gl3 Example");
    bgui::set_up_gl3();
    bgui::set_up_freetype();
    {
        bgui::scoped_interface interface_scope;
        auto& sm = bgui::style_manager::get_instance();
        sm.apply_theme(bgui::dark_theme());

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
        auto panel_text = panel->add<bgui::text>("Linear Layout Example\nYou can add more widgets here.", 0.4f);
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
        window_text->style.layout.align = bgui::vec<2UL, bgui::alignment>({bgui::alignment::center, bgui::alignment::start});
        
        auto chkbx = context->add<bgui::checkbox>("Allow the checkbox above", 0.4f, true);
        chkbx->style.visual.visible = true;
       
        auto btn = context->add<bgui::button>("Button inside window", 0.4f, [](){});
        btn->style.visual.visible = true;
        auto fi = context->add<bgui::field>("UI Scale", "1", 0.4f);
        auto fii = context->add<bgui::field>("InputBox", "", 0.4f);
        auto& fipt = fi->get_inputbox();
        fipt.set_input_mode(bgui::input_mode::number);
        fipt.set_min_float(0.6);
        fipt.set_max_float(1.6);
        fipt.set_float_callback(bgui::set_global_scale);
        auto fiii = context->add<bgui::inputbox>("", "", 0.4f);
        fi->style.visual.visible=true;
        fii->style.visual.visible=true;
        fiii->style.visual.visible=true;

        bgui::get_context().m_refresh_func = [&](){
            bgui::glfw_update(bgui::get_context());
            bgui::load_font_queue();
            bgui::on_update();
            bgui::gl3_clear();
            bgui::gl3_render(bgui::get_draw_data());
            glfwSwapBuffers(window);
        };

        bgui::glfw_main_loop();
    }

    // Cleanup
    bgui::shutdown_gl3();
    bgui::shutdown_freetype();
    bgui::shutdown_glfw();
    return 0;
}
