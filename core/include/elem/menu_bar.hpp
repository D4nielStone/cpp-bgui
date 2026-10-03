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
        explicit menu_bar(bgui::linear& root);

        bgui::button& add_menu(
            const std::string& name,
            const std::function<void()>& fctn
        );

        bgui::context_menu& add_button(
            const std::string& name
        );

    private:
        bgui::linear& m_root;
        std::vector<std::unique_ptr<bgui::context_menu>> m_menus;
    };

}