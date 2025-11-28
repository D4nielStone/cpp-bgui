#pragma once
#include "elem/text.hpp"
#include "utils/draw.hpp"
#include <functional>

namespace belem {
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
        void get_requests(butil::draw_data& calls) override;
        void apply_theme(const butil::theme& t) override;
    };
} // namespace belem