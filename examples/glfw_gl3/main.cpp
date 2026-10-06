#include <bgui.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "elem/menu_bar.hpp"

namespace {
    struct async_progress_state {
        std::atomic<float> progress{0.f};
        std::atomic_bool modal_open{true};
        std::atomic_bool cancel{false};
    };

    class progress_task_manager {
    public:
        void start(bgui::layout& root) {
            auto& dialog = root.add_persistent<bgui::modal, bgui::layer::overlay>();
            dialog.add_persistent<bgui::text>(
                "Working in the background without blocking the UI.",
                0.4f
            );
            auto& progress = dialog.add_persistent<bgui::progress_bar>(0.f, 100.f);
            progress.style.layout.require_width(bgui::mode::match_parent);
            progress.style.layout.require_height(bgui::mode::pixel, 20.f);

            auto state = std::make_shared<async_progress_state>();
            dialog.set_on_confirm([state]() {
                state->modal_open.store(false, std::memory_order_release);
            });
            m_tasks.push_back({state, &progress});
            m_workers.emplace_back([state]() {
                using clock = std::chrono::steady_clock;
                constexpr auto duration = std::chrono::seconds(3);
                const auto start = clock::now();
                auto elapsed = clock::duration::zero();

                while (elapsed < duration &&
                       !state->cancel.load(std::memory_order_acquire)) {
                    elapsed = clock::now() - start;
                    const float fraction =
                        std::chrono::duration<float>(elapsed).count() /
                        std::chrono::duration<float>(duration).count();
                    state->progress.store(
                        std::min(100.f, fraction * 100.f),
                        std::memory_order_relaxed
                    );
                    std::this_thread::sleep_for(std::chrono::milliseconds(16));
                }

                if (!state->cancel.load(std::memory_order_acquire))
                    state->progress.store(100.f, std::memory_order_relaxed);
            });
        }

        void update() {
            for (const auto& task : m_tasks) {
                if (task.state->modal_open.load(std::memory_order_acquire))
                    task.bar->set_value(
                        task.state->progress.load(std::memory_order_relaxed)
                    );
            }
        }

        void cancel_and_join() {
            for (const auto& task : m_tasks)
                task.state->cancel.store(true, std::memory_order_release);
            for (auto& worker : m_workers) {
                if (worker.joinable())
                    worker.join();
            }
        }

    private:
        struct task_ui {
            std::shared_ptr<async_progress_state> state;
            bgui::progress_bar* bar;
        };

        std::vector<task_ui> m_tasks;
        std::vector<std::thread> m_workers;
    };
}

int main() {
    // Setup
    GLFWwindow* window = bgui::set_up_glfw(1280, 720, "BGUI GLFW & gl3 Example");
    bgui::set_up_gl3();
    bgui::set_up_freetype();
    progress_task_manager progress_tasks;
    {
        bgui::scoped_interface interface_scope;
        auto& sm = bgui::style_manager::get_instance();
        sm.apply_theme(bgui::dark_theme());
        bgui::set_logs_enabled(false);
        bgui::enable_proffiling(true);
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
        auto task_button = panel.add<bgui::button>(
            "Run async task (3s)",
            0.4f,
            [&root, &progress_tasks]() { progress_tasks.start(root); }
        );
        task_button->style.layout.require_mode(bgui::mode::match_parent, bgui::mode::wrap_content);
        auto panel_text = panel.add<bgui::text>("Linear Layout Example\nYou can add more widgets here.", 0.4f);
        auto tree = panel.add<bgui::tree>("Project");
        auto& source_tree = tree->add_child("src");
        source_tree.add_child("main.cpp");
        source_tree.add_child("ui.cpp");
        auto& assets_tree = tree->add_child("assets");
        assets_tree.add_child("theme.json");
        tree->set_expanded(true);
        source_tree.set_expanded(true);
        assets_tree.set_expanded(true);
        auto window_text = demo_window.add<bgui::text>("This is a window widget example.", 0.4f);
        window_text->style.layout.align = bgui::vec<2UL, bgui::alignment>({bgui::alignment::center, bgui::alignment::start});

        auto& win2 = dock->add_window("Vector Field Example", bgui::dock_area::bottom);
        auto vector2_field = win2.add<bgui::vector_field>(
            "Vec2", std::vector<std::string>{"x:", "y:"}
        );
        auto vector3_field = win2.add<bgui::vector_field>(
            "Vec3", std::vector<std::string>{"x:", "y:", "z:"}
        );
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
            progress_tasks.update();
            bgui::load_font_queue();
            bgui::on_update();
            bgui::gl3_clear();
            bgui::gl3_render(bgui::get_draw_data());
            glfwSwapBuffers(window);
        };

        bgui::glfw_main_loop();
        progress_tasks.cancel_and_join();
        bgui::save_configuration("ui.cfg");
    }
    // Cleanup
    bgui::shutdown_gl3();
    bgui::shutdown_freetype();
    bgui::shutdown_glfw();
    return 0;
}
