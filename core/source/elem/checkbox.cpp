#include "elem/checkbox.hpp"
#include "elem/button.hpp"

bgui::checkbox::checkbox(bool* ref, bool actv, const std::string& title, const float scale) : m_ref(ref), m_active(actv), linear(bgui::orientation::horizontal) {
    m_visible = false;
    recives_input(true);

    // box should be a button
    auto& box = add<bgui::element>();
    if(title.empty() == false) {
        auto& txt = add<text>(title, scale);
        txt.set_margin(0, 2);
        txt.recives_input(false);
        txt.request_width(mode::stretch);
        txt.set_alignment(alignment::end);
    }
    
    box.recives_input(false);
    request_width(mode::match_parent);
    request_height(mode::wrap_content);
    box.set_margin(5, 5);
    box.request_height(mode::match_parent);
    box.request_width(mode::same);
}

void bgui::checkbox::on_clicked() {
    m_active = m_active ? false : true;
    get_elements()[0]->get_material().set("bg_color", color{0.1f, 0.2f, 0.1f, (float)m_active});
    if(m_ref != nullptr) *m_ref = m_active;
}