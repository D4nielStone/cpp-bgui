
#include "lay/linear.hpp"
#include "bgui.hpp"
#include <iostream>
using namespace bgui;
    
linear::linear(const orientation& ori) : m_orientation(ori) {
    m_visible = false;
}

void linear::update() {
    if (m_elements.empty()) return;

    const bool vertical = (m_orientation == orientation::vertical);
    const int main = vertical ? 1 : 0;
    const int cross = vertical ? 0 : 1;

    // Padding
    int pad_main_start  = vertical ? m_padding[1] : m_padding[0];
    int pad_main_end    = vertical ? m_padding[3] : m_padding[2];
    int pad_cross_start = vertical ? m_padding[0] : m_padding[1];
    int pad_cross_end   = vertical ? m_padding[2] : m_padding[3];

    // Available size inside padding
    vec2i available = processed_size();
    available[0] -= (m_padding[0] + m_padding[2]);
    available[1] -= (m_padding[1] + m_padding[3]);

    // ─────────────────────────────
    // PREPASS — measure fixed items
    // ─────────────────────────────
    float fixed_main = 0.f;
    int stretch_count = 0;

    for (auto& elem : m_elements) {
        auto mode = elem->get_requested_mode()[main];

        if (mode == mode::pixel || mode == mode::wrap_content || mode == mode::same) {
            elem->update_size(available);

            fixed_main += elem->processed_size()[main];
            fixed_main += elem->get_padding()[main];
            fixed_main += elem->get_padding()[main + 2];
        } else {
            stretch_count++;
        }
    }

    // ─────────────────────────────
    // STRETCH calculation
    // ─────────────────────────────
    float stretch_size = 0.f;
    if (stretch_count > 0) {
        stretch_size = (available[main] - fixed_main) / stretch_count;
        if (stretch_size < 0.f) stretch_size = 0.f;
    }

    // Apply stretch sizes
    for (auto& elem : m_elements) {
        auto mode = elem->get_requested_mode()[main];

        if (mode == mode::stretch || mode == mode::match_parent) {
            if (vertical) {
                elem->update_size({ available[0], (int)stretch_size });
            } else {
                elem->update_size({ (int)stretch_size, available[1] });
            }
        }
    }

    // ─────────────────────────────
    // TOTAL content size (for align)
    // ─────────────────────────────
    int content_main = 0;
    for (auto& elem : m_elements) {
        content_main += elem->get_padding()[main];
        content_main += elem->processed_size()[main];
        content_main += elem->get_padding()[main + 2];
    }

    int free_space = available[main] - content_main;
    if (free_space < 0) free_space = 0;

    // ─────────────────────────────
    // Initial cursor (alignment)
    // ─────────────────────────────
    int cursor_main = pad_main_start;

    switch (m_alignment) {
        case alignment::start:
            break;

        case alignment::center:
            cursor_main += free_space / 2;
            break;

        case alignment::end:
            cursor_main += free_space;
            break;
    }

    // ─────────────────────────────
    // FINAL PASS — place elements
    // ─────────────────────────────
    for (auto& elem : m_elements) {
        // margin before
        cursor_main += elem->get_margin()[main];

        int cross_pos = pad_cross_start + elem->get_margin()[cross];
        int cross_size = elem->processed_size()[cross];

        switch (m_cross_alignment) {
            case alignment::start:
                break;

            case alignment::center:
                cross_pos = pad_cross_start +
                            (available[cross] - cross_size) / 2 +
                            elem->get_margin()[cross] -
                            elem->get_margin()[cross + 2];
                break;

            case alignment::end:
                cross_pos = processed_size()[cross]
                            - pad_cross_end
                            - cross_size
                            - elem->get_margin()[cross + 2];
                break;
        }

        if (vertical) {
            elem->set_final_rect(
                cross_pos + processed_x(),
                cursor_main + processed_y(),
                elem->processed_width(),
                elem->processed_height()
            );
        } else {
            elem->set_final_rect(
                cursor_main + processed_x(),
                cross_pos + processed_y(),
                elem->processed_width(),
                elem->processed_height()
            );
        }

        // advance cursor
        cursor_main += elem->processed_size()[main];
        cursor_main += elem->get_margin()[main + 2];

        elem->update();
    }
}

float linear::content_height() {
    if (m_elements.empty()) return 0.f;

    bool vertical = (m_orientation == orientation::vertical);
    int total = 0;

    for (auto& elem : m_elements) {
        int h = elem->processed_height() +
                elem->get_margin()[1] +
                elem->get_margin()[3];

        if (vertical)
            total += h;
        else
            total = std::max(total, h);
    }

    return total;
}


float bgui::linear::content_width() {
    if (m_elements.empty()) return 0.f;

    const bool vertical = (m_orientation == orientation::vertical);
    int total_width = 0;

    for (auto& elem : m_elements) {
        total_width += elem->processed_width();
        if (!vertical) {
            total_width += elem->get_padding()[0] + elem->get_padding()[2];
        } else {
            // In horizontal layout, consider only the largest height
            total_width = std::max(total_width, elem->processed_height() + elem->get_padding()[0] + elem->get_padding()[2]);
        }
    }

    return total_width;
}