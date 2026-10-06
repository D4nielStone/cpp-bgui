#include <bgui.hpp>

#include <iostream>
#include <string>
#include <vector>

int main() {
    GLFWwindow* window = bgui::set_up_glfw(
        1440,
        900,
        "cpp-bgui | OpenGL API showcase"
    );
    bgui::set_up_gl3();
    bgui::set_up_freetype();

    {
        bgui::scoped_interface interface_scope;
        auto& styles = bgui::style_manager::get_instance();
        styles.apply_theme(bgui::dark_theme());
        bgui::set_font_antialiasing(true);

        auto& root = bgui::set_layout<bgui::linear>(
            bgui::orientation::vertical
        );
        auto& menu_bar = root.add_persistent<bgui::menu_bar>(root);
        menu_bar.style.layout.require_height(
            bgui::mode::wrap_content
        );

        auto& workspace = root.add_persistent<bgui::dock>();
        workspace.style.layout.require_mode(
            bgui::mode::match_parent,
            bgui::mode::stretch
        );

        auto& gallery = workspace.add_window(
            "Widget gallery",
            bgui::dock_area::center
        );
        auto& floating = workspace.add_persistent<bgui::window>(
            "Floating window",
            true
        );
        floating.set_position(770, 95);

        auto& status = gallery.add_persistent<bgui::text>(
            "Ready — drag the dock dividers or window tabs to rearrange the workspace.",
            0.35f
        );
        status.style.layout.require_mode(
            bgui::mode::match_parent,
            bgui::mode::wrap_content
        );

        gallery.add_persistent<bgui::text>(
            "Widgets",
            0.55f
        );
        gallery.add_persistent<bgui::text>(
            "Buttons, fields, editable text, selectors, progress indicators, "
            "images, and nested layouts.",
            0.36f
        );

        auto& click_count = gallery.add_persistent<bgui::text>(
            "Button clicks: 0",
            0.38f
        );
        int button_clicks = 0;
        auto& action_button = gallery.add_persistent<bgui::button>(
            "Click me",
            0.4f,
            [&click_count, &button_clicks]() {
                ++button_clicks;
                click_count.set_buffer(
                    "Button clicks: " + std::to_string(button_clicks)
                );
            }
        );
        action_button.id = "showcase-action";
        action_button.add_class("primary");

        auto& theme_toggle = gallery.add_persistent<bgui::checkbox>(
            "Dark theme",
            0.38f,
            true
        );
        theme_toggle.set_on_change([&styles](bool dark) {
            styles.apply_theme(dark ? bgui::dark_theme() : bgui::light_theme());
        });
        auto& enable_toggle = gallery.add_persistent<bgui::checkbox>(
            "Enable theme switch",
            0.38f,
            true
        );
        enable_toggle.set_on_change([&theme_toggle](bool enabled) {
            theme_toggle.set_enable(enabled);
        });

        auto& selection_text = gallery.add_persistent<bgui::text>(
            "Selected: OpenGL",
            0.38f
        );
        auto& renderer_combo = gallery.add_persistent<bgui::combo_box>(
            std::vector<std::string>{"OpenGL", "Vulkan", "Custom"},
            0.38f
        );
        renderer_combo.set_on_change(
            [&selection_text](std::size_t, const std::string& option) {
                selection_text.set_buffer("Selected: " + option);
            }
        );

        auto& progress_label = gallery.add_persistent<bgui::text>(
            "Progress: 35%",
            0.38f
        );
        auto& progress = gallery.add_persistent<bgui::progress_bar>(
            0.f,
            100.f,
            35.f
        );
        progress.style.layout.require_width(bgui::mode::match_parent);
        progress.style.layout.require_height(bgui::mode::pixel, 14.f);
        auto& progress_slider = gallery.add_persistent<bgui::slider>(
            0.f,
            100.f,
            35.f
        );
        progress_slider.style.layout.require_width(bgui::mode::match_parent);
        progress_slider.style.layout.require_height(bgui::mode::pixel, 24.f);
        progress_slider.set_step(5.f);
        progress_slider.set_on_change(
            [&progress, &progress_label](float value) {
                progress.set_value(value);
                progress_label.set_buffer(
                    "Progress: " + std::to_string(static_cast<int>(value)) + "%"
                );
            }
        );

        auto& color_label = gallery.add_persistent<bgui::text>(
            "Color picker (click or drag inside the palette):",
            0.35f
        );
        color_label.style.layout.require_mode(
            bgui::mode::match_parent,
            bgui::mode::wrap_content
        );
        auto& color_picker = gallery.add_persistent<bgui::color_picker>(
            bgui::color{0.2f, 0.55f, 0.9f, 1.f}
        );
        color_picker.style.layout.require_size(150.f, 110.f);
        color_picker.set_on_change([&status](const bgui::color& color) {
            status.set_buffer(
                "Color: " +
                std::to_string(static_cast<int>(color.r * 255.f)) + ", " +
                std::to_string(static_cast<int>(color.g * 255.f)) + ", " +
                std::to_string(static_cast<int>(color.b * 255.f))
            );
        });

        gallery.add_persistent<bgui::field>(
            "Name",
            "",
            0.36f,
            [&status](const std::string& value) {
                status.set_buffer("Submitted name: " + value);
            },
            "Type a name and press Enter"
        );
        auto& editor = gallery.add_persistent<bgui::inputbox>(
            "print('Hello from cpp-bgui')\n",
            "Multiline editor",
            0.34f,
            nullptr,
            bgui::input_mode::multiline
        );
        editor.style.layout.require_mode(
            bgui::mode::match_parent,
            bgui::mode::pixel
        );
        editor.style.layout.require_height(bgui::mode::pixel, 90.f);
        editor.set_wrap(true);
        editor.set_editor_guides_enabled(true);
        gallery.use_highlight_config("lua_highlight.json");

        auto& position = gallery.add_persistent<bgui::vector_field>(
            "Position",
            std::vector<std::string>{"X", "Y", "Z"},
            0.34f
        );
        position.style.layout.require_width(bgui::mode::match_parent);

        auto& about = gallery.add_persistent<bgui::details>(
            "Details / collapsible content",
            true
        );
        about.content().add_persistent<bgui::text>(
            "Details can show or hide nested content.",
            0.34f
        );

        auto& modal = root.add_persistent<
            bgui::modal,
            bgui::layer::overlay
        >();
        modal.add_persistent<bgui::text>(
            "This modal is an overlay. Confirm to close it.",
            0.38f
        );
        modal.set_confirmation_text("Close");
        modal.set_on_confirm([&status]() {
            status.set_buffer("Modal confirmed and closed.");
        });
        modal.set_enable(false);
        gallery.add_persistent<bgui::button>(
            "Show modal",
            0.38f,
            [&modal]() {
                modal.set_enable(true);
            }
        );

        auto& image = gallery.add_persistent<bgui::image>(
            "bubble.png",
            [&status](const std::string& path) {
                status.set_buffer("Image clicked: " + path);
            }
        );
        image.set_size(110.f, 70.f);
        image.set_size_mode(bgui::mode::pixel, bgui::mode::pixel);

        auto& config_menu = menu_bar.add_button("Layout");
        config_menu.add_item(
            "Save dock layout",
            0.34f,
            [&status]() {
                const std::string path = "bgui-showcase-layout.ini";
                if (bgui::save_configuration(path)) {
                    status.set_buffer("Dock layout saved to " + path);
                } else {
                    status.set_buffer("Could not save layout to " + path);
                    std::cerr << "Could not save BGUI layout to " << path << '\n';
                }
            }
        );
        config_menu.add_item(
            "Load dock layout",
            0.34f,
            [&status]() {
                const std::string path = "bgui-showcase-layout.ini";
                if (bgui::load_configuration(path)) {
                    status.set_buffer("Dock layout loaded from " + path);
                } else {
                    status.set_buffer("Could not load layout from " + path);
                    std::cerr << "Could not load BGUI layout from " << path << '\n';
                }
            }
        );
        config_menu.add_separator();
        config_menu.add_button("Close menu");

        menu_bar.add_menu(
            "Theme",
            0.36f,
            [&styles](bgui::context_menu& menu) {
                menu.add_item("Dark", 0.34f, [&styles]() {
                    styles.apply_theme(bgui::dark_theme());
                });
                menu.add_item("Light", 0.34f, [&styles]() {
                    styles.apply_theme(bgui::light_theme());
                });
            }
        );

        menu_bar.add_menu("About", [&status]() {
            status.set_buffer(
                "cpp-bgui OpenGL showcase — widgets, layouts, themes, and docking."
            );
        });

        floating.add_persistent<bgui::text>(
            "This window can be dragged, resized, docked, or closed.",
            0.36f
        );
        floating.add_persistent<bgui::button>(
            "Focus gallery",
            0.36f,
            [&workspace, &gallery]() {
                workspace.focus_window(&gallery);
            }
        );

        while (!bgui::should_close_glfw()) {
            glfwPollEvents();
            bgui::glfw_update(bgui::get_context());
            bgui::on_update();
            bgui::load_font_queue();
            bgui::gl3_clear();
            bgui::gl3_render(bgui::get_draw_data());
            bgui::swap_glfw();
        }
    }

    bgui::shutdown_freetype();
    bgui::shutdown_gl3();
    bgui::shutdown_glfw();
    return 0;
}
