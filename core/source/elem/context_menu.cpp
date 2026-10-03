#include "elem/menu_bar.hpp"

namespace bgui {
    context_menu::context_menu(
        bgui::linear& parent,
        bgui::button& button
    )
        : m_parent(parent),
        m_button(button) {

        m_menu = &m_parent.add_persistent<
            bgui::linear,
            bgui::layer::overlay
        >(bgui::orientation::vertical);

        m_menu->set_flex(false);

        m_menu->style.layout.require_mode(
            bgui::mode::pixel,
            bgui::mode::pixel
        );

        m_menu->style.layout.require_size(
            190.f,
            0.f
        );

        m_menu->set_enable(false);
    }

    context_menu& context_menu::add_button(
        const std::string& name,
        const std::function<void()>& fctn
    ) {
        auto& button = m_menu->add_persistent<bgui::button>(
            name,
            0.35f,
            [this, fctn]() {
                fctn();
                close();
            }
        );

        button.style.layout.require_mode(
            bgui::mode::match_parent,
            bgui::mode::wrap_content
        );

        button.add_class("context-menu-button");

        return *this;
    }

    context_menu& context_menu::add_button(
        const std::string& name
    ) {
        return add_button(name, []() {});
    }

    void context_menu::open(int x, int y) {
        if (!m_menu)
            return;

        m_menu->set_position(x, y);
        m_menu->set_enable(true);
        m_menu->set_flex(false);
    }

    void context_menu::close() {
        if (!m_menu)
            return;

        m_menu->set_enable(false);
    }
    void context_menu::toggle() {
        if (!m_menu)
            return;

        const bool open = !m_menu->is_enabled();

        if (open) {
            m_menu->set_position(
                m_button.processed_x(),
                m_button.processed_y() +
                    m_button.processed_height()
            );

            m_menu->set_enable(true);
            m_menu->set_flex(false);
        } else {
            m_menu->set_enable(false);
        }
    }
    bool context_menu::is_open() const {
        return m_menu && m_menu->is_enabled();
    }
}