#pragma once

#include "lay/linear.hpp"
#include "elem/button.hpp"
#include "context_menu.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace bgui {
    class menu_bar : public bgui::linear {
    public:
        menu_bar();
        explicit menu_bar(bgui::layout& root);

        bgui::button& add_menu(
            const std::string& name,
            const std::function<void()>& fctn
        );
        bgui::button& add_menu(
            const std::string& name,
            float scale,
            const std::function<void(bgui::context_menu&)>& configure
        );

        bgui::context_menu& add_button(
            const std::string& name
        );

    private:
        bgui::layout& get_root() const;

        bgui::layout* m_root = nullptr;
        std::vector<std::unique_ptr<bgui::context_menu>> m_menus;
    };

    using menu = context_menu;
    using menubar = menu_bar;
}