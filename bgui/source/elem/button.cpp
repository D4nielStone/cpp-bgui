#include "elem/button.hpp"
#include "bgui.hpp"
#include <iostream>

elements::button::button(const std::string &name, const float scale, const std::function<void()> &f) : 
    m_label(name, scale), m_function(f) {
    m_alignment = butil::alignment::center;
    apply_theme(bgui::instance().get_theme());
    m_material.m_shader.compile("quad.vs", "quad.fs");
}

elements::button::~button() {
}

void elements::button::update() {
    m_material.set("u_bg_color", bgui::instance().get_theme().m_button_color);
    set_size(m_label.get_width() + m_intern_spacing[0]*2, m_label.get_height() + m_intern_spacing[1]*2);
}

void elements::button::get_draw_requests(std::vector<butil::draw_request> &calls) {
    element::get_draw_requests(calls);
    switch(m_alignment) {
        case butil::alignment::start:
            m_label.set_position(get_x() + m_intern_spacing[0], get_y() + m_intern_spacing[1]);
            break;
        case butil::alignment::center:
            m_label.set_position(
                get_x() + (get_width() - m_label.get_width()) / 2,
                get_y() + (get_height() - m_label.get_height()) / 2
            );
            break;
        case butil::alignment::end:
            m_label.set_position(
                get_x() + get_width() - m_label.get_width() - m_intern_spacing[0],
                get_y() + get_height() - m_label.get_height() - m_intern_spacing[1]
            );
            break;
        default:
            break;
    }
    m_label.get_draw_requests(calls);
}

void elements::button::apply_theme(const butil::theme &t) {
    m_material.set("u_bg_color", t.m_button_color);
    m_material.set("u_bordered", true);
    m_material.set("u_border_radius", 4.f);
    m_material.set("u_border_size", 1.f);
    m_material.set("u_border_color", t.m_button_border_color);
    m_material.m_visible = true;
}

void elements::button::on_released() {
    bgui::instance().add_gl_call(m_function);
}
void elements::button::on_clicked() {
    m_material.set("u_bg_color", bgui::instance().get_theme().m_button_clicked_color);
}
void elements::button::on_mouse_hover() {
    m_material.set("u_bg_color", bgui::instance().get_theme().m_button_hovered_color);
}