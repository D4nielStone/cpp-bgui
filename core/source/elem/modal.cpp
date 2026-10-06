#include "elem/modal.hpp"

#include "bgui.hpp"
#include "os/os.hpp"

namespace bgui {
    modal::modal() : linear(orientation::vertical) {
        bgui::clear_keyboard_focus();
        type = "modal";
        recives_input(true);
        style.layout.require_mode(mode::match_parent, mode::match_parent);
        style.layout.align = vec<2UL, alignment>({alignment::center, alignment::center});
        style.visual.background.normal = color{0.f, 0.f, 0.f, 0.65f};
        style.visual.border.normal = color{0.f, 0.f, 0.f, 0.f};

        set_position(0, 0);

        m_panel = &linear::add_persistent<linear>(orientation::vertical);
        m_panel->style.layout.require_mode(mode::wrap_content, mode::wrap_content);
        m_panel->style.layout.set_padding(16, 16);
        m_panel->style.visual.background.normal = color{0.12f, 0.12f, 0.12f, 1.f};
        m_panel->style.visual.border.normal = color{0.32f, 0.32f, 0.32f, 1.f};
        m_panel->style.visual.border_size = 1.f;
        m_panel->style.visual.border_radius = 4.f;
        m_panel->style.visual.visible = true;

        m_content = &m_panel->add_persistent<linear>(orientation::vertical);
        m_content->style.layout.require_mode(mode::wrap_content, mode::wrap_content);

        m_actions = &m_panel->add_persistent<linear>(orientation::horizontal);
        m_actions->style.layout.require_mode(mode::stretch, mode::wrap_content);
        m_actions->style.layout.align = vec<2UL, alignment>(
            {alignment::end, alignment::start}
        );

        m_confirm_button = &m_actions->add_persistent<button>(
            "Confirmar",
            0.4f,
            [this]() { confirm(); }
        );
        m_confirm_button->style.layout.require_mode(mode::wrap_content, mode::wrap_content);
    }

    void modal::set_on_confirm(std::function<void()> callback) {
        m_on_confirm = std::move(callback);
    }

    void modal::set_confirmation_text(const std::string& text) {
        m_confirm_button->get_label().set_buffer(text);
    }

    void modal::confirm() {
        if (m_on_confirm)
            m_on_confirm();

        auto* parent = get_parent();
        if (parent) {
            bgui::add_function([parent, this]() {
                parent->remove(this);
            });
        }
    }
}
