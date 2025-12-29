#pragma once
#include "elem/text.hpp"
#include "lay/linear.hpp"
#include "utils/draw.hpp"
#include <functional>

namespace bgui {
    class checkbox : public linear {
    private:
        bool m_active, *m_ref;
        text *label{nullptr};
    public:
        color m_active_color {0.5f, 0.5f, 0.5f, 1.f};
        /// @brief Simple constructor
        /// @param id Will be processed as a global variable and accessible anytime.
        checkbox(bool* ref, bool actv = false, const std::string& title = "", const float scale = 1.f);
        ~checkbox() = default;
        
        void on_clicked() override;
        void on_released() override;
        void on_pressed() override;
        void on_update() override;
        void on_mouse_hover() override;

        text& get_label();
        bool checked() {
            return m_active;
        }
    };
} // namespace bgui