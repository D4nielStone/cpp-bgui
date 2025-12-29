#include "elem/button.hpp"
#include "os/os.hpp"
#include "bgui.hpp"
#include <iostream>

bgui::button::button(const std::string &name, const float scale, const std::function<void()> &f) : 
    m_label(&add<text>(name, scale)), m_function(f), linear(bgui::orientation::horizontal) {
    m_visible = true;
    m_label->recives_input(false);
    recives_input(true);
    request_width(mode::wrap_content);
    request_height(mode::wrap_content);
}

bgui::button::~button() {
}

void bgui::button::set_function(const std::function<void()>& f) {
    m_function = f;
}

void bgui::button::on_update() {
    m_material.set("bg_color", m_style.m_button_color);
    m_material.set("border_color", m_style.m_button_border_color);
    linear::on_update();
}

bgui::text &bgui::button::get_label() {
    return *m_label;
}
void bgui::button::on_pressed() {
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
}

void bgui::button::on_released() {
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
    bgui::add_function(m_function);
}
void bgui::button::on_clicked() {
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
    if(get_style().m_button_clicked_color[3] != 0)
        m_material.set("bg_color", bgui::get_style().m_button_clicked_color);
}
void bgui::button::on_mouse_hover() {
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
    if(get_style().m_button_hovered_color[3] != 0)
        m_material.set("bg_color", bgui::get_style().m_button_hovered_color);
}