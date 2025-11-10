#include "elem/linear_layout.hpp"
#include "bgui.hpp"
    
linear_layout::linear_layout(const orientation& ori) {
    m_orientation = ori;
}

void linear_layout::update() {
    int cursor_x = 0, cursor_y = 0;

    for (auto& elem : m_elements) {
        elem->update();
        // vertical
        if(m_orientation == orientation::vertical) {
            // start
            if(m_alignment == alignment::start) {
                elem->set_position(cursor_x, cursor_y);
            } else if (m_alignment == alignment::center) {
            // center
                // normal
                // cross
                cursor_x = get_width()/2 - elem->get_width()/2;
                elem->set_position(cursor_x, cursor_y);
            }
            cursor_y += elem->get_height();
        }
    }
};

void linear_layout::get_draw_calls(std::vector<draw_call>& calls) {
    element::get_draw_calls(calls);
    // linear layouts get the draw call in addition order
    for (auto& elem : m_elements) {
        elem->get_draw_calls(calls);
    }
};