#pragma once
#include "elem/text.hpp"
#include "utils/draw.hpp"
#include <functional>

namespace bgui {
    class button : public element {
    private:
        std::function<void()> m_function;
        text m_label;
        alignment m_label_alignment;
    public:
        button(const std::string& name, const float scale, const std::function<void()>& f);
        ~button();
        
        void on_clicked() override;
        void on_released() override;
        void on_mouse_hover() override;
        float content_width() override;
        float content_height() override;
        void update() override;
        void get_requests(bgui::draw_data* calls) override;
        void apply_style(const bgui::style& t) override;
    };
} // namespace bgui