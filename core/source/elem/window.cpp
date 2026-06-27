#include "elem/window.hpp"
#include "elem/button.hpp"
#include "os/os.hpp"

bgui::window::window(const char* title, bool floating) : linear(bgui::orientation::vertical), m_title(nullptr), m_header(nullptr) {
    type = "window";
    // window widget experiment
    //TODO:: add parse init config for window
    set_position(20, 20);

    // testing the header:
    m_header = &add<bgui::linear>(bgui::orientation::horizontal);
    m_header->add_class("window-header");
    m_title = &m_header->add<bgui::text>(title, 0.35f);
    m_title->add_class("window-label");
    // TODO: switch to image button later
    m_header->add<bgui::button>(" X ", 0.35f, [this](){
        m_parent->remove(this);
    }).add_class("window-button");
    set_floating(floating);
}
void bgui::window::on_update() {
    // drag system (title)
    if(is_floating()) {
        if(m_title->is_drag()[0] || m_title->is_drag()[1]) {
            set_position(processed_x()+m_title->is_drag()[0], processed_y()+m_title->is_drag()[1]);
        }
    }
    m_title->set_drag({0, 0});
    linear::on_update();
}
void bgui::window::set_floating(bool floating) {
    m_floating = floating;
    set_flex(!floating);
    if(m_header) {
        auto* elem = m_header->get_elements_by_class("window-button")[0];
        elem->set_enable(floating);
    }
}