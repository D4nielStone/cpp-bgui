#pragma once

#include "elem/button.hpp"
#include "elem/text.hpp"
#include "lay/linear.hpp"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace bgui {
    class combo_box : public linear {
    public:
        explicit combo_box(
            const std::vector<std::string>& options,
            float scale = 0.4f,
            std::size_t selected_index = 0
        );
        ~combo_box() override;

        std::size_t get_selected_index() const noexcept {
            return m_selected_index;
        }
        const std::string& get_selected_option() const noexcept {
            return m_options[m_selected_index];
        }
        void set_selected_index(std::size_t index);
        void set_on_change(
            const std::function<void(std::size_t, const std::string&)>& callback
        );

        void on_clicked() override;
        void on_pressed() override;
        void on_released() override;
        void on_mouse_hover() override;
        void on_update() override;

    private:
        void ensure_options_menu();
        void select_option(std::size_t index);
        void toggle_options();

        std::vector<std::string> m_options;
        float m_scale;
        std::size_t m_selected_index{0};
        text* m_selected_label{nullptr};
        linear* m_options_menu{nullptr};
        layout* m_overlay_parent{nullptr};
        std::function<void(std::size_t, const std::string&)> m_on_change;
    };
}
