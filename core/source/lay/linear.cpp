
#include "lay/linear.hpp"
#include "bgui.hpp"
#include <iostream>
using namespace bgui;
    
linear::linear(const orientation& ori) {
    m_orientation = ori;
    m_visible = false;
    m_material.m_shader_tag = "ui::default";
}

void linear::update() {
    if (m_elements.empty()) return;
    for(auto& elem : m_elements) elem->update();

    const bool vertical = (m_orientation == orientation::vertical);
    const int main_index = vertical ? 1 : 0;
    const int cross_index = vertical ? 0 : 1;

    const int spacing = m_spacing_elements[main_index];

    // padding on main axis (start and end)
    int pad_main_start = (vertical ? m_padding[1] : m_padding[0]);
    int pad_main_end = (vertical ? m_padding[3] : m_padding[2]);
    // Available size on main axis (respect padding)
    const int available_main = get_size()[main_index] - (pad_main_start + pad_main_end);

    int elem_total_main = 0;

    // Sum child sizes + spacing
    for (auto& elem : m_elements) {
        elem_total_main += elem->get_size()[main_index];
    }

    if (m_elements.size() > 1)
        elem_total_main += spacing * int(m_elements.size() - 1);

    // Starting cursor depending on alignment
    int cursor_main = 0;

    switch (m_alignment) {
        case alignment::start:
            cursor_main = pad_main_start;
            break;

        case alignment::center:
            cursor_main = (available_main - elem_total_main) / 2 + pad_main_start;
            break;

        case alignment::end:
            cursor_main = available_main - elem_total_main + pad_main_start;
            break;
        default:
            break;
    }

    // Place elem
    for (auto& elem : m_elements) {

        int cross_pos = 0;

        const int cross_size = elem->get_size()[cross_index];
        const int container_cross = get_size()[cross_index];

        int pad_cross_start = (!vertical ? m_padding[1] : m_padding[0]);
        int pad_cross_end = (!vertical ? m_padding[3] : m_padding[2]);

        // Cross-axis alignment
        switch (m_cross_alignment) {
            case alignment::start:
                cross_pos = pad_cross_start;
                break;

            case alignment::center:
                cross_pos = (container_cross - cross_size) / 2;
                break;

            case alignment::end:
                cross_pos = container_cross - cross_size - pad_cross_end;
                break;

            case alignment::stretch:
                cross_pos = pad_cross_start;
                if (vertical)
                    elem->set_width(container_cross - (pad_cross_start + pad_cross_end));
                else
                    elem->set_height(container_cross - (pad_cross_start + pad_cross_end));
                break;
        }

        // Set position
        if (vertical) {
            elem->set_x(cross_pos + get_x());
            elem->set_y(cursor_main + get_y());
        } else {
            elem->set_x(cursor_main + get_x());
            elem->set_y(cross_pos + get_y());
        }

        // Advance in main axis (add child margin on the main axis if any)
        unsigned int child_margin_start = vertical ? elem->get_margin()[1] : elem->get_margin()[0];
        unsigned int child_margin_end = vertical ? elem->get_margin()[3] : elem->get_margin()[2];
        cursor_main += elem->get_size()[main_index] + spacing + child_margin_start + child_margin_end;
    }
}

void linear::fit_to_content() {
    const bool vertical = (m_orientation == orientation::vertical);
    int max_cross = 0;
    for (auto& elem : m_elements) {
        int cross = vertical ? elem->get_width() : elem->get_height();
        if (cross > max_cross)
            max_cross = cross;
    }
    int pad_cross_sum = vertical ? (m_padding[0] + m_padding[2]) : (m_padding[1] + m_padding[3]);
    int max_main = vertical ? (m_padding[1] + m_padding[3]) : (m_padding[0] + m_padding[2]);
    for (auto& elem : m_elements) {
        max_main += vertical ? elem->get_height() : elem->get_width();
        auto mg = elem->get_margin();
        max_main += vertical ? (mg[1] + mg[3]) : (mg[0] + mg[2]);
    }

    if (vertical) {
        set_width(max_cross + pad_cross_sum);
        set_height(max_main);
    } else {
        set_height(max_cross + pad_cross_sum);
        set_width(max_main);
    }
}
