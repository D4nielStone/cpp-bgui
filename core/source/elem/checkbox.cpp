#include "elem/checkbox.hpp"
#include "elem/button.hpp"
#include "os/os.hpp"

bgui::checkbox::checkbox(bool* ref, bool actv, const std::string& title, const float scale) : m_ref(ref), m_active(actv), linear(bgui::orientation::horizontal) {
    type = "checkbox";
    recives_input(true);
    // box should be a button
    auto& box = add<bgui::element>();
    box.add_class("checkbox-box");
    if(title.empty() == false) {
        auto& txt = add<text>(title, scale);
        txt.add_class("checkbox-txt");
        label = &txt;
        txt.recives_input(false);
    }
    
    box.recives_input(false);
}
bgui::checkbox::checkbox(const std::string& title, const float scale, const bool actv) : m_ref(nullptr), m_active(actv), linear(bgui::orientation::horizontal) {
    type = "checkbox";

    recives_input(true);

    // box should be a button
    auto& box = add<bgui::element>();
    box.add_class("checkbox-box");

    if(title.empty() == false) {
        auto& txt = add<text>(title, scale);
        txt.add_class("checkbox-txt");
        label = &txt;
        txt.recives_input(false);
    }
    
    box.recives_input(false);
}

void bgui::checkbox::on_clicked() {
    element::on_clicked();
    bgui::get_context().m_actual_cursor = cursor::hand;
    m_active = m_active ? false : true;
    if(m_ref != nullptr) *m_ref = m_active;
    if(m_function) m_function(m_active);
}

void bgui::checkbox::set_on_change(const std::function<void(bool)>& f) {
    m_function = f;
    auto& box = get_elements()[0];
    m_active ? box->set_style_state(state::pressed) : box->set_style_state(state::normal);
}

void bgui::checkbox::on_released() {
    element::on_released();
    bgui::get_context().m_actual_cursor = cursor::hand;
}
void bgui::checkbox::on_pressed() {
    element::on_pressed();
    bgui::get_context().m_actual_cursor = cursor::hand;
}
void bgui::checkbox::on_mouse_hover() {
    element::on_mouse_hover();
    bgui::get_context().m_actual_cursor = cursor::hand;
}

void bgui::checkbox::on_update() {
    linear::on_update();
    auto& box = get_elements()[0];
    m_active ? box->set_style_state(state::pressed) : box->set_style_state(state::normal);
}

bgui::text& bgui::checkbox::get_label() {
    return *label;
}