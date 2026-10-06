#include "elem/menu_bar.hpp"

namespace bgui {
    context_menu::context_menu(
        bgui::layout& parent,
        bgui::button& button
    )
        : m_button(button),
          m_parent(parent) {

        m_menu = &m_parent.add_persistent<
            bgui::linear,
            bgui::layer::overlay
        >(bgui::orientation::vertical);

        m_menu->add_class("combo-box-popup");
        m_menu->set_flex(false);

        m_menu->style.layout.require_mode(
            bgui::mode::wrap_content,
            bgui::mode::wrap_content
        );
        m_menu->style.layout.limit_min = bgui::vec2i{200, 10};
        m_menu->style.visual.visible = true;

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

        button.add_class("combo-box-option");

        return *this;
    }

    context_menu& context_menu::add_button(
        const std::string& name
    ) {
        return add_button(name, []() {});
    }

    context_menu& context_menu::add_item(
        const std::string& name,
        float scale,
        const std::function<void()>& fctn
    ) {
        auto& button = m_menu->add_persistent<bgui::button>(
            name,
            scale,
            [this, fctn]() {
                fctn();
                close();
            }
        );
        button.add_class("combo-box-option");

        return *this;
    }

    context_menu& context_menu::add_separator() {
        auto& separator = m_menu->add_persistent<bgui::element>();
        separator.style.layout.require_mode(
            bgui::mode::match_parent,
            bgui::mode::pixel
        );
        separator.style.layout.require_height(bgui::mode::pixel, 1.f);
        separator.style.layout.limit_min = bgui::vec2i{1, 1};
        separator.style.visual.background.normal =
            bgui::color{0.35f, 0.35f, 0.35f, 1.f};

        return *this;
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