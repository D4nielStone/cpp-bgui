#pragma once

#include "lay/linear.hpp"
#include "elem/button.hpp"

namespace bgui {
    class context_menu {
    public:
        context_menu(bgui::layout& parent, bgui::button& btn);

        context_menu& add_button(
            const std::string& name,
            const std::function<void()>& fctn
        );

        context_menu& add_button(const std::string& name);
        context_menu& add_item(
            const std::string& name,
            float scale,
            const std::function<void()>& fctn
        );
        context_menu& add_separator();

        void open(int x, int y);
        void close();
        void toggle();

        bool is_open() const;

    private:
        bgui::button& m_button;
        bgui::layout& m_parent;
        bgui::linear* m_menu = nullptr;
    };
}