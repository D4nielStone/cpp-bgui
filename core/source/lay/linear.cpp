#include "lay/linear.hpp"
#include "utils/enums.hpp"
#include "bgui.hpp"
#include <algorithm>

using namespace bgui;

linear::linear(const orientation& ori) : m_orientation(ori), layout() {
    type = "linear";
}

void linear::on_update() {
    element::on_update();
    calc_content_size();

    if (m_elements.empty()) return;

    const bool vertical = (m_orientation == orientation::vertical);
    const int main = vertical ? 1 : 0;
    const int cross = vertical ? 0 : 1;

    int pad_main_start  = vertical ? computed_style.layout.padding.y : computed_style.layout.padding.x;
    int pad_main_end    = vertical ? computed_style.layout.padding.w : computed_style.layout.padding.z;
    int pad_cross_start = vertical ? computed_style.layout.padding.x : computed_style.layout.padding.y;
    int pad_cross_end   = vertical ? computed_style.layout.padding.z : computed_style.layout.padding.w;

    vec2i available = processed_size();
    available.x -= (computed_style.layout.padding.x + computed_style.layout.padding.z);
    available.y -= (computed_style.layout.padding.y + computed_style.layout.padding.w);

    float fixed_main = 0.f;
    int stretch_count = 0;

    for (auto& elem : m_elements) {
        auto mreq = elem->computed_style.layout.size_mode[main];

        if (mreq == mode::pixel || mreq == mode::wrap_content || mreq == mode::same) {
            elem->process_required_size(available);
            fixed_main += elem->processed_size()[main];
            fixed_main += elem->computed_style.layout.margin[main];
            fixed_main += elem->computed_style.layout.margin[main + 2];
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
        if(!elem->is_enabled()) continue;
        vec2i final_available = elem->processed_size();

        for (int axis = 0; axis < 2; ++axis) {
            auto req = elem->computed_style.layout.size_mode[axis];

            int margin_before = elem->computed_style.layout.margin[axis];
            int margin_after  = elem->computed_style.layout.margin[axis + 2];

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
                        static_cast<int>(axis_available);
                    break;

                case mode::pixel:
                case mode::wrap_content:
                case mode::same:
                default:
                    break;
            }
        }

        final_available.x = std::max(0, final_available.x);
        final_available.y = std::max(0, final_available.y);

        elem->process_required_size(final_available);
    }

    int content_main = 0;
    for (auto& elem : m_elements) {
        if(!elem->is_enabled()) continue;
        content_main += elem->computed_style.layout.margin[main];
        content_main += elem->processed_size()[main];
        content_main += elem->computed_style.layout.margin[main + 2];
    }

    content_main += pad_main_start + pad_main_end;

    int free_space = available[main] - (content_main - pad_main_start - pad_main_end);
    if (free_space < 0) free_space = 0;

    int cursor_main = pad_main_start;

    switch (computed_style.layout.align.x) {
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
        if(!elem->is_enabled()) continue;

        cursor_main += elem->computed_style.layout.margin[main];

        int cross_pos = pad_cross_start + elem->computed_style.layout.margin[cross];
        int cross_size = elem->processed_size()[cross];

        switch (computed_style.layout.align.y) {
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
                cross_pos = pad_cross_start + ((available[cross] - cross_size) - elem->computed_style.layout.margin[cross + 2]);
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
        cursor_main += elem->computed_style.layout.margin[main + 2];

        elem->on_update();
    }
}

void linear::calc_content_size() {
    if(!is_style_dirty()) return;
    const bool vertical = (m_orientation == orientation::vertical);

    int content_w = 0;
    int content_h = 0;

    // Padding
    const int pad_left   = computed_style.layout.padding.x;
    const int pad_top    = computed_style.layout.padding.y;
    const int pad_right  = computed_style.layout.padding.z;
    const int pad_bottom = computed_style.layout.padding.w;

    if (m_elements.empty()) {
        m_content_size = {
            pad_left + pad_right,
            pad_top + pad_bottom
        };
        return;
    }

    for (auto& elem : m_elements) {
        if (!elem->is_enabled())
            continue;

        const auto& margin = elem->computed_style.layout.margin;

        const int elem_w =
            elem->processed_width() +
            margin.x + margin.z;

        const int elem_h =
            elem->processed_height() +
            margin.y + margin.w;

        if (vertical) {
            content_h += elem_h;
            content_w = std::max(content_w, elem_w);
        } else {
            content_w += elem_w;
            content_h = std::max(content_h, elem_h);
        }
    }

    content_w += pad_left + pad_right;
    content_h += pad_top + pad_bottom;

    set_content_size({ content_w, content_h });
}