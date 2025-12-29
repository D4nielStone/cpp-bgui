#include "lay/linear.hpp"
#include "bgui.hpp"
#include <algorithm>

using namespace bgui;

linear::linear(const orientation& ori) : m_orientation(ori) {
    m_visible = false;
}

void linear::on_update() {
    if (m_elements.empty()) return;

    const bool vertical = (m_orientation == orientation::vertical);
    const int main = vertical ? 1 : 0;
    const int cross = vertical ? 0 : 1;

    int pad_main_start  = vertical ? m_padding[1] : m_padding[0];
    int pad_main_end    = vertical ? m_padding[3] : m_padding[2];
    int pad_cross_start = vertical ? m_padding[0] : m_padding[1];
    int pad_cross_end   = vertical ? m_padding[2] : m_padding[3];

    vec2i available = processed_size();
    available[0] -= (m_padding[0] + m_padding[2]);
    available[1] -= (m_padding[1] + m_padding[3]);

    float fixed_main = 0.f;
    int stretch_count = 0;

    for (auto& elem : m_elements) {
        auto mreq = elem->get_requested_mode()[main];

        if (mreq == mode::pixel || mreq == mode::wrap_content || mreq == mode::same) {
            elem->update_size(available);
            fixed_main += elem->processed_size()[main];
            fixed_main += elem->get_margin()[main];
            fixed_main += elem->get_margin()[main + 2];
        } else {
            stretch_count++;
        }
    }

    float stretch_size = 0.f;
    if (stretch_count > 0) {
        stretch_size = (available[main] - fixed_main) / stretch_count;
        if (stretch_size < 0.f) stretch_size = 0.f;
    }

    for (auto& elem : m_elements) {
        vec2i final_available = elem->processed_size();

        for (int axis = 0; axis < 2; ++axis) {
            auto req = elem->get_requested_mode()[axis];

            int margin_before = elem->get_margin()[axis];
            int margin_after  = elem->get_margin()[axis + 2];

            int axis_available = available[axis] - margin_before - margin_after;
            axis_available = std::max(0, axis_available);

            switch (req) {
                case mode::stretch:
                    if (axis == main) {
                        final_available[axis] = static_cast<int>(stretch_size);
                    }
                    break;

                case mode::match_parent:
                    final_available[axis] = axis_available;
                    break;

                case mode::percent:
                    final_available[axis] =
                        static_cast<int>(axis_available * elem->requested_size()[axis]);
                    break;

                case mode::pixel:
                case mode::wrap_content:
                case mode::same:
                default:
                    break;
            }
        }

        final_available[0] = std::max(0, final_available[0]);
        final_available[1] = std::max(0, final_available[1]);

        elem->update_size(final_available);
    }

    int content_main = 0;
    for (auto& elem : m_elements) {
        content_main += elem->get_margin()[main];
        content_main += elem->processed_size()[main];
        content_main += elem->get_margin()[main + 2];
    }

    content_main += pad_main_start + pad_main_end;

    int free_space = available[main] - (content_main - pad_main_start - pad_main_end);
    if (free_space < 0) free_space = 0;

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

    for (auto& elem : m_elements) {
        cursor_main += elem->get_margin()[main];

        int cross_pos = pad_cross_start + elem->get_margin()[cross];
        int cross_size = elem->processed_size()[cross];

        switch (m_cross_alignment) {
            case alignment::start:
                break;

            case alignment::center:
                {
                    int total_cross_occupied = cross_size;
                    int extra = available[cross] - total_cross_occupied;
                    if (extra < 0) extra = 0;
                    cross_pos = pad_cross_start + extra / 2;
                }
                break;

            case alignment::end:
                cross_pos = pad_cross_start + (available[cross] - cross_size - elem->get_margin()[cross + 2]);
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

        cursor_main += elem->processed_size()[main];
        cursor_main += elem->get_margin()[main + 2];

        elem->on_update();
    }
}

float linear::content_height() {
    if (m_elements.empty()) {
        return m_padding[1] + m_padding[3];
    }

    const bool vertical = (m_orientation == orientation::vertical);
    int total = 0;

    for (auto& elem : m_elements) {
        int h = elem->processed_height() +
                elem->get_margin()[1] +
                elem->get_margin()[3];

        if (vertical) {
            total += h;
        } else {
            total = std::max(total, h);
        }
    }

    total += m_padding[1] + m_padding[3];

    return static_cast<float>(total);
}

float bgui::linear::content_width() {
    if (m_elements.empty()) {
        return m_padding[0] + m_padding[2];
    }

    const bool vertical = (m_orientation == orientation::vertical);
    int total_width = 0;

    for (auto& elem : m_elements) {
        int w = elem->processed_width() +
                elem->get_margin()[0] +
                elem->get_margin()[2];

        if (!vertical) {
            total_width += w;
        } else {
            total_width = std::max(total_width, w);
        }
    }

    total_width += m_padding[0] + m_padding[2];

    return static_cast<float>(total_width);
}