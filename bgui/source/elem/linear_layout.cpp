#include "elem/linear_layout.hpp"
#include "bgui.hpp"
    
linear_layout::linear_layout(const orientation& ori) {
    m_orientation = ori;
}

void linear_layout::update() {
    // no children -> nothing to do
    if (m_elements.empty()) return;

    const bool vertical = (m_orientation == orientation::vertical);

    // compute total main-axis size (sum of widths or heights) + spacing between items
    int spacing = vertical ? m_spacing_elements[1] : m_spacing_elements[0];
    int total_main = 0;
    for (size_t i = 0; i < m_elements.size(); ++i) {
        total_main += (vertical ? m_elements[i]->get_height() : m_elements[i]->get_width());
    }
    if (m_elements.size() > 1) total_main += spacing * (int(m_elements.size()) - 1);

    // compute starting cursor on the main axis depending on alignment (start/center/end)
    int cursor_main;
    if (vertical) {
        if (m_alignment == alignment::start)
            cursor_main = get_y();
        else if (m_alignment == alignment::center)
            cursor_main = get_y() + (get_height() - total_main) / 2;
        else // end
            cursor_main = get_y() + get_height() - total_main;
    } else {
        if (m_alignment == alignment::start)
            cursor_main = get_x();
        else if (m_alignment == alignment::center)
            cursor_main = get_x() + (get_width() - total_main) / 2;
        else // end
            cursor_main = get_x() + get_width() - total_main;
    }

    // Place each child using cursor_main and cross-axis alignment
    for (auto& elem : m_elements) {
        elem->update();
        // child already updated above
        int ex = get_x();
        int ey = get_y();

        if (vertical) {
            // cross-axis (horizontal) alignment for this child
            if (m_cross_alignment == alignment::start)
                ex = get_x();
            else if (m_cross_alignment == alignment::center)
                ex = get_x() + (get_width() - elem->get_width()) / 2;
            else // end
                ex = get_x() + get_width() - elem->get_width();

            // main-axis position = cursor_main (y)
            ey = cursor_main;

            // commit positions
            elem->set_x(ex);
            elem->set_y(ey);

            // advance cursor
            cursor_main += elem->get_height() + spacing + elem->m_extern_spacing[1];
        } else {
            // cross-axis (vertical) alignment for this child
            if (m_cross_alignment == alignment::start)
                ey = get_y();
            else if (m_cross_alignment == alignment::center)
                ey = get_y() + (get_height() - elem->get_height()) / 2;
            else // end
                ey = get_y() + get_height() - elem->get_height();

            // main-axis position = cursor_main (x)
            ex = cursor_main;

            // commit positions
            elem->set_x(ex);
            elem->set_y(ey);

            // advance cursor
            cursor_main += elem->get_width() + spacing + elem->m_extern_spacing[0];
        }
    }
}


void linear_layout::get_draw_calls(std::vector<draw_call>& calls) {
    element::get_draw_calls(calls);
    // linear layouts get the draw call in addition order
    for (auto& elem : m_elements) {
        elem->get_draw_calls(calls);
    }
};