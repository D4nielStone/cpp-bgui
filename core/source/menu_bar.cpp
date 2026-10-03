#include "elem/menu_bar.hpp"

namespace bgui {
    bgui::menu_bar::menu_bar(bgui::linear& root)
    : bgui::linear(bgui::orientation::horizontal),
      m_root(root) {

        add_class("window-header");

        style.layout.require_mode(
            bgui::mode::match_parent,
            bgui::mode::wrap_content
        );

        style.layout.padding = bgui::vec4i{
            12,
            0,
            8,
            0
        };

        auto& title =
            add_persistent<bgui::text>(
                " Bubble Engine ",
                0.35f
            );

        title.style.layout.require_mode(
            bgui::mode::wrap_content,
            bgui::mode::wrap_content
        );
    }
    context_menu& menu_bar::add_button(const std::string& name) {
        m_menus.push_back(nullptr);

        auto& menu_ptr = m_menus.back();

        auto& button = add_persistent<bgui::button>(
            name,
            0.35f,
            [&menu_ptr]() {
                if (menu_ptr)
                    menu_ptr->toggle();
            }
        );

        button.add_class("window-button");

        button.style.layout.require_mode(
            bgui::mode::wrap_content,
            bgui::mode::match_parent
        );

        button.style.layout.set_margin(4, 0);

        button.style.visual.background.hover =
            bgui::color{0.17f, 0.17f, 0.17f, 1.f};

        menu_ptr = std::make_unique<context_menu>(
            m_root,
            button
        );

        return *menu_ptr;
    }
    bgui::button& menu_bar::add_menu(
        const std::string& name,
        const std::function<void()>& fctn
    ) {
        auto& button = add_persistent<bgui::button>(
            name,
            0.35f,
            fctn
        );

        button.add_class("window-button");

        button.style.layout.require_mode(
            bgui::mode::wrap_content,
            bgui::mode::match_parent
        );

        button.style.layout.set_margin(4, 0);

        button.style.visual.background.hover =
            bgui::color{0.17f, 0.17f, 0.17f, 1.f};

        return button;
    }
}