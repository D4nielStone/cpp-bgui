
#include "lay/linear.hpp"
#include "bgui.hpp"
#include <iostream>
using namespace bgui;
    
linear::linear(const orientation& ori) : m_orientation(ori) {
    m_visible = false;
}

void linear::update() {
    if (m_elements.empty()) return;

    vec2i avalaible_size = processed_size() - vec2i({
        m_padding[0] + m_padding[2],
        m_padding[1] + m_padding[3]
    });
    int m_stretch_count = 0;
    float total_fixed_size = 0.f;


    const bool vertical = (m_orientation == orientation::vertical);
    // compute available size along MAIN direction
    const int main_index = vertical ? 1 : 0;
    const int cross_index = vertical ? 0 : 1;

    int stretch_count = 0;
    float fixed_total = 0;

    // PREPASS: measure non-stretch elements
    for (auto& elem : m_elements) {
        auto mode = elem->get_requested_mode()[main_index];

        if (mode == mode::pixel || mode == mode::wrap_content) {
            elem->update_size(avalaible_size);
            fixed_total += elem->processed_size()[main_index];
        }
        else if (mode == mode::stretch || mode == mode::match_parent) {
            stretch_count++;
        }
    }

    // compute stretch size
    float stretch_size = 0.f;
    if (stretch_count > 0) {
        stretch_size = (avalaible_size[main_index] - fixed_total) / stretch_count;
        if (stretch_size < 0.f) stretch_size = 0.f;
    }

    // SECOND PASS: assign stretch sizes
    for (auto& elem : m_elements) {
        auto mode = elem->get_requested_mode()[main_index];

        if (mode == mode::stretch || mode == mode::match_parent) {
            if (vertical) {
                elem->update_size({avalaible_size[0], (int)stretch_size});
            } else {
                elem->update_size({(int)stretch_size, avalaible_size[1]});
            }
        }
    }

    // padding on main axis (start and end)
    int pad_main_start = (vertical ? m_padding[1] : m_padding[0]);
    int pad_main_end = (vertical ? m_padding[3] : m_padding[2]);
    // Available size on main axis (respect padding)
    const int available_main = processed_size()[main_index] - (pad_main_start + pad_main_end);

    int elem_total_main = 0;

    // Sum child sizes + spacing
    for (auto& elem : m_elements) {
        elem_total_main += elem->processed_size()[main_index] + elem->get_margin()[main_index] + elem->get_margin()[main_index+2];
    }

    // Starting cursor depending on alignment
    int cursor_main = 0;

    switch (m_alignment) {
        case alignment::start:
            cursor_main = pad_main_start + m_elements.begin()->get()->get_margin()[main_index];
            break;

        case alignment::center:
            cursor_main = (available_main - elem_total_main) / 2;
            break;

        case alignment::end:
            cursor_main = pad_main_start + available_main - elem_total_main - m_elements.begin()->get()->get_margin()[main_index+2];
            break;
        default:
            break;
    }

    // Place elem
    for (auto& elem : m_elements) {

        int cross_pos = 0;

        const int cross_size = elem->processed_size()[cross_index];
        const int container_cross = processed_size()[cross_index];

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
        }

        // Set position
        if (vertical) {
            elem->set_final_rect(cross_pos + processed_x(), cursor_main + processed_y(), elem->processed_width(), elem->processed_height());
        } else {
            elem->set_final_rect(cursor_main + processed_x(), cross_pos + processed_y(), elem->processed_width(), elem->processed_height());
        }

        // Advance in main axis (add child margin on the main axis if any)
        cursor_main += elem->processed_size()[main_index] + elem->get_margin()[main_index+2];
        // finally update the element. that function is mostly used by layouts.
        elem->update();
    }
}

float bgui::linear::content_height() {
    if (m_elements.empty()) return 0.f;

    const bool vertical = (m_orientation == orientation::vertical);
    int total_height = 0;

    for (auto& elem : m_elements) {
        if (vertical) {
            total_height += elem->get_margin()[1] + elem->get_margin()[3] +  elem->processed_height();
        } else {
            // In horizontal layout, consider only the largest height
            total_height = std::max(total_height, elem->processed_height() + elem->get_margin()[1] + elem->get_margin()[3]);
        }
    }

    return total_height;
}


float bgui::linear::content_width() {
    if (m_elements.empty()) return 0.f;

    const bool vertical = (m_orientation == orientation::vertical);
    int total_width = 0;

    for (auto& elem : m_elements) {
        total_width += elem->processed_width();
        if (!vertical) {
            total_width += elem->get_margin()[0] + elem->get_margin()[2];
        } else {
            // In horizontal layout, consider only the largest height
            total_width = std::max(total_width, elem->processed_height() + elem->get_margin()[0] + elem->get_margin()[2]);
        }
    }

    return total_width;
}