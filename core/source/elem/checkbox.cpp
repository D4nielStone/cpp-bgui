#include "elem/checkbox.hpp"
#include "elem/button.hpp"
#include "os/os.hpp"

bgui::checkbox::checkbox(bool* ref, bool actv, const std::string& title, const float scale) : m_ref(ref), m_active(actv), linear(bgui::orientation::horizontal) {
    m_visible = true;
    recives_input(true);

    // box should be a button
    auto& box = add<bgui::element>();
    if(title.empty() == false) {
        auto& txt = add<text>(title, scale);
        label = &txt;
        txt.set_margin(0, 2);
        txt.recives_input(false);
        txt.request_width(mode::stretch);
        txt.set_alignment(alignment::end);
    }
    
    box.recives_input(false);
    request_width(mode::match_parent);
    request_height(mode::wrap_content);
    box.set_margin(10, 10);
    box.request_height(mode::match_parent);
    box.request_width(mode::same);
}

void bgui::checkbox::on_clicked() {
    bgui::get_context().m_actual_cursor = cursor::hand;
    m_active = m_active ? false : true;
    if(m_ref != nullptr) *m_ref = m_active;
}
void bgui::checkbox::on_released() {
    bgui::get_context().m_actual_cursor = cursor::hand;
}
void bgui::checkbox::on_pressed() {
    bgui::get_context().m_actual_cursor = cursor::hand;
}
void bgui::checkbox::on_mouse_hover() {
    bgui::get_context().m_actual_cursor = cursor::hand;
}

void bgui::checkbox::on_update() {
    linear::on_update();
    m_active_color[3] = m_active ? 1.f : 0.f;
    auto bc = m_active_color/2;
    bc[3] = 1.f;

    get_material().set("bg_color", get_style().m_cb_color);
    get_material().set("border_color", get_style().m_cb_border_color);
    get_elements()[0]->get_material().set("bg_color", m_active_color);
    get_elements()[0]->get_material().set("border_color", bc);
}

bgui::text& bgui::checkbox::get_label() {
    return *label;
}