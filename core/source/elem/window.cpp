#include "elem/window.hpp"
#include "elem/button.hpp"

bgui::window::window(const char* title) : linear(bgui::orientation::vertical), m_title(nullptr), m_header(nullptr) {
    // window widget experiment
    //TODO:: add parse init config for window
    set_position(20, 20);
    request_size(300, 300);
    set_visible(true);

    // testing the header:
    m_header = &add<bgui::linear>(bgui::orientation::horizontal);
    m_header->request_height(bgui::mode::wrap_content);
    m_header->request_width(bgui::mode::match_parent);
    // TODO: switch to icon image later
    m_header->add<bgui::button>(" V ", 0.5f, [](){});
    m_title = &m_header->add<bgui::text>(title, 0.5f);
    m_title->request_width(bgui::mode::stretch);
    m_title->set_alignment(bgui::alignment::center);
    // TODO: switch to image button later
    m_header->add<bgui::button>(" X ", 0.5f, [this](){
        m_parent->remove(this);
    });
    m_header->set_visible(true);
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