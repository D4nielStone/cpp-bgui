#pragma once
#include "elem/text.hpp"
#include "lay/linear.hpp"
#include "utils/draw.hpp"
#include <functional>

namespace bgui {
    class checkbox : public linear {
    private:
        bool m_active, *m_ref;
    public:
        /// @brief Simple constructor
        /// @param id Will be processed as a global variable and accessible anytime.
        checkbox(bool* ref, bool actv = false, const std::string& title = "", const float scale = 1.f);
        ~checkbox() = default;
        
        void on_clicked() override;
        //void on_released() override;
        //void on_mouse_hover() override;
        //float content_width() override;
        //float content_height() override;
        //void update() override;

        bool checked() {
            return m_active;
        }
    };
} // namespace bgui