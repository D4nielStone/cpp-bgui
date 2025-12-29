#pragma once
#include "elem/text.hpp"
#include "lay/linear.hpp"
#include "utils/draw.hpp"
#include <functional>

namespace bgui {
    class button : public linear {
    private:
        std::function<void()> m_function;
        text *m_label;
    public:
        button(const std::string& name, const float scale, const std::function<void()>& f);
        ~button();
        
        void set_function(const std::function<void()>& f);
        void on_clicked() override;
        void on_pressed() override;
        void on_released() override;
        void on_mouse_hover() override;
        text& get_label();
        void update() override;
    };
} // namespace bgui