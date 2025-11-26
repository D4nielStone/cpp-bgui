#pragma once
#include "elem/text.hpp"
#include <functional>

namespace elements {
    class button : public element {
    private:
        std::function<void()> m_function;
        text m_label;
    public:
        button(const std::string& name, const float scale, const std::function<void()>& f);
        ~button();
        
        void on_clicked() override;
        void on_released() override;
        void on_mouse_hover() override;
        void update() override;
        void get_draw_requests(std::vector<butil::draw_request>& calls) override;
        void apply_theme(const butil::theme& t) override;
    };
} // namespace elements