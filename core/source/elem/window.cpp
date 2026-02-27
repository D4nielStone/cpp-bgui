#include "elem/window.hpp"
#include "elem/button.hpp"
#include "os/os.hpp"

bgui::window::window(const char* title) : linear(bgui::orientation::vertical), m_title(nullptr), m_header(nullptr) {
    type = "window";
    // window widget experiment
    //TODO:: add parse init config for window
    set_position(20, 20);

    // testing the header:
    m_header = &add<bgui::linear>(bgui::orientation::horizontal);
    m_header->add_class("window-header");
    // TODO: switch to icon image later
    m_header->add<bgui::button>(" V ", 0.35f, [](){}).add_class("window-button");
    m_title = &m_header->add<bgui::text>(title, 0.35f);
    m_title->add_class("window-label");
    // TODO: switch to image button later
    m_header->add<bgui::button>(" X ", 0.35f, [this](){
        m_parent->remove(this);
    }).add_class("window-button");
}
void bgui::window::on_update() {
    // drag system (title)
    set_final_rect(
        processed_x()+m_title->is_drag()[0], 
        processed_y()+m_title->is_drag()[1],
        processed_width(), 
        processed_height());
    linear::on_update();
}