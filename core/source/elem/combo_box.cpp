#include "elem/combo_box.hpp"

#include "bgui.hpp"
#include "os/os.hpp"

#include <stdexcept>

bgui::combo_box::combo_box(
    const std::vector<std::string>& options,
    float scale,
    std::size_t selected_index
) : m_options(options),
    m_scale(scale),
    m_selected_index(selected_index),
    linear(bgui::orientation::horizontal) {
    if (m_options.empty())
        throw std::invalid_argument("[BGUI] Combo box requires at least one option.");
    if (m_selected_index >= m_options.size())
        throw std::out_of_range("[BGUI] Combo box selected index is out of range.");

    type = "combobox";
    recives_input(true);

    m_selected_label = &add_persistent<text>(
        m_options[m_selected_index],
        scale
    );
    m_selected_label->add_class("combo-box-label");
    m_selected_label->recives_input(false);

    auto& indicator = add_persistent<text>("v", scale);
    indicator.add_class("combo-box-indicator");
    indicator.recives_input(false);
}

bgui::combo_box::~combo_box() {
    if (m_options_menu && m_overlay_parent)
        m_overlay_parent->remove(m_options_menu);
}

void bgui::combo_box::ensure_options_menu() {
    if (m_options_menu)
        return;

    m_overlay_parent = this;
    while (m_overlay_parent->get_parent())
        m_overlay_parent = m_overlay_parent->get_parent();

    m_options_menu = &m_overlay_parent->add_persistent<linear, layer::overlay>(
        bgui::orientation::vertical
    );
    m_options_menu->add_class("combo-box-popup");
    m_options_menu->style.visual.visible = true;
    m_options_menu->style.layout.require_mode(
        bgui::mode::match_parent,
        bgui::mode::wrap_content
    );
    m_options_menu->set_flex(false);
    m_options_menu->set_enable(false);

    for (std::size_t index = 0; index < m_options.size(); ++index) {
        auto& option = m_options_menu->add_persistent<button>(
            m_options[index],
            m_scale,
            [this, index]() {
                select_option(index);
                m_options_menu->set_enable(false);
            }
        );
        option.add_class("combo-box-option");
        option.style.layout.require_mode(bgui::mode::match_parent, bgui::mode::wrap_content);
    }
}

void bgui::combo_box::set_selected_index(std::size_t index) {
    if (index >= m_options.size())
        throw std::out_of_range("[BGUI] Combo box selected index is out of range.");
    select_option(index);
}

void bgui::combo_box::set_on_change(
    const std::function<void(std::size_t, const std::string&)>& callback
) {
    m_on_change = callback;
}

void bgui::combo_box::select_option(std::size_t index) {
    if (index == m_selected_index)
        return;

    m_selected_index = index;
    m_selected_label->set_buffer(m_options[m_selected_index]);
    if (m_on_change)
        m_on_change(m_selected_index, m_options[m_selected_index]);
}

void bgui::combo_box::toggle_options() {
    ensure_options_menu();

    const bool open = !m_options_menu->is_enabled();
    m_options_menu->set_enable(open);
    if (!open)
        return;

    m_options_menu->cascade_style();
    m_options_menu->set_position(
        processed_x(),
        processed_y() + processed_height()
    );
}

void bgui::combo_box::on_clicked() {
    element::on_clicked();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
    toggle_options();
}

void bgui::combo_box::on_pressed() {
    element::on_pressed();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
}

void bgui::combo_box::on_released() {
    element::on_released();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
}

void bgui::combo_box::on_mouse_hover() {
    element::on_mouse_hover();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
}

void bgui::combo_box::on_update() {
    linear::on_update();
    ensure_options_menu();

    if (m_options_menu->is_enabled()) {
        const auto& context = bgui::get_context();
        const bool clicked = bgui::get_pressed(bgui::input_key::mouse_left) &&
            !context.m_last_mouse_left;
        if (clicked) {
            const auto mouse = bgui::get_mouse_position();
            const bool inside_combo =
                mouse.x >= processed_x() &&
                mouse.x <= processed_x() + processed_width() &&
                mouse.y >= processed_y() &&
                mouse.y <= processed_y() + processed_height();
            const bool inside_options =
                mouse.x >= m_options_menu->processed_x() &&
                mouse.x <= m_options_menu->processed_x() + m_options_menu->processed_width() &&
                mouse.y >= m_options_menu->processed_y() &&
                mouse.y <= m_options_menu->processed_y() + m_options_menu->processed_height();
            if (!inside_combo && !inside_options)
                m_options_menu->set_enable(false);
        }
    }

    m_options_menu->calc_content_size(layer::base);
    m_options_menu->process_required_size(processed_size());
    m_options_menu->style.layout.require_width(
        bgui::mode::pixel,
        static_cast<float>(processed_width()) / bgui::get_global_scale()
    );
    m_options_menu->mark_style_dirty();
    m_options_menu->compute_style();
    m_options_menu->set_position(
        processed_x(),
        processed_y() + processed_height()
    );
}
