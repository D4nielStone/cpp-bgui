#include <bgui.hpp>
#include <iostream>
#include "elem/menu_bar.hpp"

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
        auto workspace = root.add<bgui::linear>(bgui::orientation::vertical);
        workspace->style.layout.require_mode(
            bgui::mode::match_parent, bgui::mode::match_parent
        );
        auto menubar = workspace->add<bgui::menubar>(root);
        menubar->style.layout.require_height(
            bgui::mode::wrap_content, 1.f
        );
        menubar->add_menu("File", 0.4f, [](bgui::menu& menu) {
            menu.add_item("New", 0.4f, []() { std::cout << "New clicked\n"; });
            menu.add_item("Open", 0.4f, []() { std::cout << "Open clicked\n"; });
            menu.add_item("Save", 0.4f, []() { std::cout << "Save clicked\n"; });
            menu.add_separator();
            menu.add_item("Exit", 0.4f, []() { std::cout << "Exit clicked\n"; });
        });
        auto dock = workspace->add<bgui::dock>();
        auto& demo_window = dock->add_window("Hello Bubble!");
        auto& panel = dock->add_window("Panel", bgui::dock_area::right);
        auto panel_text = panel.add<bgui::text>("Linear Layout Example\nYou can add more widgets here.", 0.4f);
        auto window_text = demo_window.add<bgui::text>("This is a window widget example.", 0.4f);
        window_text->style.layout.align = bgui::vec<2UL, bgui::alignment>({bgui::alignment::center, bgui::alignment::start});

        auto value_text = demo_window.add<bgui::text>(
            "Slider and progress bar", 0.4f);
        auto value_slider = demo_window.add<bgui::slider>(
            -50.f, 50.f, 0, bgui::orientation::horizontal);
        value_slider->style.layout.require_width(bgui::mode::match_parent);
        value_slider->style.layout.require_height(bgui::mode::wrap_content);
        value_slider->set_step(5.f);
        value_text->set_buffer("Slider value: " + std::to_string(value_slider->get_value()));
        value_slider->set_on_change([label = &value_text.get()](float value) {
            label->set_buffer("Slider value: " + std::to_string(value));
        });

        auto combo_label = demo_window.add<bgui::text>(
            "Combo box: Small", 0.4f);
        auto combo = demo_window.add<bgui::combo_box>(
            std::vector<std::string>{"Small", "Medium", "Large"});
        combo->set_on_change([label = &combo_label.get()](
            std::size_t, const std::string& option) {
            label->set_buffer("Combo box: " + option);
        });

        auto color_text = demo_window.add<bgui::text>("Color: #FF0000", 0.4f);
        auto color_control = demo_window.add<bgui::color_picker>();
        color_control->style.layout.require_size(150.f, 150.f);
        color_control->set_on_change([label = &color_text.get()](const bgui::color& value) {
            const auto channel = [](float component) {
                constexpr char digits[] = "0123456789ABCDEF";
                const int byte = static_cast<int>(component * 255.f + 0.5f);
                std::string result(2, '0');
                result[0] = digits[(byte >> 4) & 0xF];
                result[1] = digits[byte & 0xF];
                return result;
            };
            label->set_buffer(
                "Color: #" + channel(value.r) + channel(value.g) + channel(value.b)
            );
        });

        auto chkbx = demo_window.add<bgui::checkbox>("CheckBox Element", 0.4f, true);

        auto btn = demo_window.add<bgui::button>("Button inside window", 0.4f, [](){});
        auto modal_button = demo_window.add<bgui::button>(
            "Open modal",
            0.4f,
            [&root]() {
                auto& dialog = root.add_persistent<bgui::modal, bgui::layer::overlay>();
                dialog.add_persistent<bgui::text>(
                    "This dialog fits its content, blocks the interface behind it,\nand stays open until you confirm.",
                    0.4f
                );
                dialog.add_persistent<bgui::button>(
                    "Open another modal",
                    0.4f,
                    [&root]() {
                        auto& stacked_dialog = root.add_persistent<bgui::modal, bgui::layer::overlay>();
                        stacked_dialog.add_persistent<bgui::text>(
                            "This modal is stacked above the previous one.",
                            0.4f
                        );
                    }
                );
            }
        );
        auto fi = demo_window.add<bgui::field>("UI Scale", "1", 0.4f);
        auto fii = demo_window.add<bgui::field>("InputBox", "", 0.4f);
        auto& fipt = fi->get_inputbox();
        fipt.set_input_mode(bgui::input_mode::number);
        fipt.set_min_float(0.6);
        fipt.set_max_float(1.6);
        fipt.set_float_callback(bgui::set_global_scale);
        auto fiii = demo_window.add<bgui::inputbox>("", "", 0.4f);

        auto& text_editor_window = dock->add_window("Text Editor", bgui::dock_area::bottom);
        auto text_editor = text_editor_window.add<bgui::inputbox>(
            "-- You can write your code below --", "", 0.4f,
            [](const std::string& text) {}, bgui::input_mode::multiline);
        text_editor->style.layout.require_mode(
            bgui::mode::match_parent, bgui::mode::match_parent);
        text_editor_window.use_highlight_config("lua_highlight.json");
        text_editor->set_wrap(false);
        text_editor->set_editor_guides_enabled(true);

        bgui::load_configuration("ui.cfg");
        bgui::get_context().m_refresh_func = [&](){
            bgui::glfw_update(bgui::get_context());
            bgui::load_font_queue();
            bgui::on_update();
            bgui::gl3_clear();
            bgui::gl3_render(bgui::get_draw_data());
            glfwSwapBuffers(window);
        };

        bgui::glfw_main_loop();
        bgui::save_configuration("ui.cfg");
    }
    // Cleanup
    bgui::shutdown_gl3();
    bgui::shutdown_freetype();
    bgui::shutdown_glfw();
    return 0;
}
