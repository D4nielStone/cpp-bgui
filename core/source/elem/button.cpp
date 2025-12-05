#include "elem/button.hpp"
#include "bgui.hpp"
#include <iostream>

bgui::button::button(const std::string &name, const float scale, const std::function<void()> &f) : 
    m_label(name, scale), m_function(f), m_label_alignment(bgui::alignment::center) {
    request_width(mode::wrap_content);
    request_height(mode::wrap_content);
}

bgui::button::~button() {
}

void bgui::button::update() {
    m_material.set("bg_color", m_theme.m_button_color);

    m_label.update_size(processed_size());
    switch(m_label_alignment) {
        case bgui::alignment::start:
            m_label.set_final_rect(processed_x() + m_padding[0], processed_y() + m_padding[1],
            m_label.processed_width(), m_label.processed_height());
            break;
        case bgui::alignment::center:
            m_label.set_final_rect(
                processed_x() + ((processed_width()) - m_label.processed_width()) / 2,
                processed_y() + ((processed_height()) - m_label.processed_height()) / 2,
                m_label.processed_width(),
                m_label.processed_height()
            );
            break;
        case bgui::alignment::end:
            m_label.set_final_rect(
                processed_x() + processed_width() - m_label.processed_width() - m_padding[0],
                processed_y() + processed_height() - m_label.processed_height() - m_padding[1],
                m_label.processed_width(),
                m_label.processed_height()
            );
            break;
        default:
            break;
    }
}

void bgui::button::get_requests(bgui::draw_data* data) {
    element::get_requests(data);
    m_label.get_requests(data);
}


float bgui::button::content_height() {
    return m_label.processed_height() + get_padding()[1] + get_padding()[3];
}

float bgui::button::content_width() {
    return m_label.processed_width() + get_padding()[0] + get_padding()[2];
}

void bgui::button::apply_theme(const bgui::theme &t) {
    element::apply_theme(t);
    m_visible = true;
    m_label.apply_theme(t);
}

void bgui::button::on_released() {
}
void bgui::button::on_clicked() {
    m_material.set("bg_color", bgui::get_theme().m_button_clicked_color);
    bgui::add_function(m_function);
}
void bgui::button::on_mouse_hover() {
    m_material.set("bg_color", bgui::get_theme().m_button_hovered_color);
}