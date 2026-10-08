#include "elem/button.hpp"
#include "os/os.hpp"
#include "bgui.hpp"
#include <iostream>

bgui::button::button(const std::string& name,
                     float scale,
                     const std::function<void()>& f)
    : m_label(&add<text>(name, scale)),
      m_function(f),
      linear(bgui::orientation::horizontal)
{
    type = "button";
    receives_input(true);
    m_label->receives_input(false);
}

bgui::button::~button() {
}

void bgui::button::set_function(const std::function<void()>& f) {
    m_function = f;
}

void bgui::button::on_update() {
    linear::on_update();
}

bgui::text &bgui::button::get_label() {
    return *m_label;
}
void bgui::button::on_pressed() {
    element::on_pressed();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
}

void bgui::button::on_released() {
    element::on_released();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
    bgui::add_function(m_function);
}
void bgui::button::on_clicked() {
    element::on_clicked();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
}
void bgui::button::on_mouse_hover() {
    element::on_mouse_hover();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
}