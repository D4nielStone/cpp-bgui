#include "elem/modal.hpp"

#include "bgui.hpp"
#include "os/os.hpp"

namespace bgui {
    modal::modal() : linear(orientation::vertical) {
        bgui::clear_keyboard_focus();
        type = "modal";
        set_flex(false);
        recives_input(true);

        set_position(0, 0);

        m_panel = &linear::add_persistent<linear>(orientation::vertical);
        m_panel->add_class("modal-panel");

        m_content = &m_panel->add_persistent<linear>(orientation::vertical);
        m_content->add_class("modal-content");

        m_actions = &m_panel->add_persistent<linear>(orientation::horizontal);
        m_actions->add_class("modal-actions");

        m_confirm_button = &m_actions->add_persistent<button>(
            "Confirmar",
            0.4f,
            [this]() { confirm(); }
        );
        m_confirm_button->add_class("modal-confirm");
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
