#include "elem/menu_bar.hpp"
#include "bgui.hpp"

#include <stdexcept>

namespace bgui {
    menu_bar::menu_bar()
    : bgui::linear(bgui::orientation::horizontal) {
        type = "menubar";
        style.layout.require_mode(
            bgui::mode::match_parent,
            bgui::mode::wrap_content
        );
        style.layout.padding = bgui::vec4i{1, 1, 8, 8};

        auto& title = add<bgui::text>(
            "© Bubble Engine | ",
            0.3
        );
        title.style.layout.require_mode(
            bgui::mode::wrap_content,
            bgui::mode::wrap_content
        );
    }

    menu_bar::menu_bar(bgui::layout& root)
    : menu_bar() {
        m_root = &root;
    }

    bgui::layout& menu_bar::get_root() const {
        if (m_root)
            return *m_root;

        if (auto* parent = get_parent())
            return *parent;

        throw std::logic_error(
            "A menu bar must be attached to a layout before adding menus."
        );
    }

    context_menu& menu_bar::add_button(const std::string& name) {
        m_menus.push_back(nullptr);
        const auto menu_index = m_menus.size() - 1;

        auto& button = add<bgui::button>(
            name,
            0.33f,
            [this, menu_index]() {
                if (m_menus[menu_index])
                    m_menus[menu_index]->toggle();
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

        m_menus[menu_index] = std::make_unique<context_menu>(
            get_root(),
            button
        );
        return *m_menus[menu_index];
    }

    bgui::button& menu_bar::add_menu(
        const std::string& name,
        const std::function<void()>& fctn
    ) {
        auto& button = add<bgui::button>(name, 0.33f, fctn);
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

    bgui::button& menu_bar::add_menu(
        const std::string& name,
        float scale,
        const std::function<void(bgui::context_menu&)>& configure
    ) {
        m_menus.push_back(nullptr);
        const auto menu_index = m_menus.size() - 1;

        auto& button = add<bgui::button>(
            name,
            scale,
            [this, menu_index]() {
                for (size_t i = 0; i < m_menus.size(); ++i) {
                    if (i != menu_index && m_menus[i])
                        m_menus[i]->close();
                }
                if (m_menus[menu_index])
                    m_menus[menu_index]->toggle();
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

        m_menus[menu_index] = std::make_unique<context_menu>(
            get_root(),
            button
        );
        if (configure)
            configure(*m_menus[menu_index]);

        return button;
    }

    void menu_bar::on_update() {
        bgui::linear::on_update();

        const auto& context = bgui::get_context();
        const bool clicked = bgui::get_pressed(bgui::input_key::mouse_left) &&
            !context.m_last_mouse_left;
        if (!clicked)
            return;

        const auto mouse = bgui::get_mouse_position();
        for (auto& menu : m_menus) {
            if (menu && menu->is_open() &&
                !menu->contains_point(mouse.x, mouse.y)) {
                menu->close();
            }
        }
    }
}