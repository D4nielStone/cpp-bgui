#include "lay/linear.hpp"
#include "utils/enums.hpp"
#include "bgui.hpp"
#include <algorithm>
#include <cmath>

using namespace bgui;

linear::linear(const orientation& ori) : m_orientation(ori), layout() {
    type = "linear";
    m_scrollbar.type = "scrollbar";
    m_scrollbar.set_parent(this);
    m_scrollbar.receives_input(true);
    m_scrollbar.style.visual.background.normal = color{0.45f, 0.45f, 0.45f, 0.85f};
    m_scrollbar.style.visual.border.normal = color{0.f, 0.f, 0.f, 0.f};
    m_scrollbar.style.visual.border_radius = 3.f;
    m_scrollbar.style.visual.border_size = 0.f;
    m_scrollbar.style.visual.visible = true;
}

void linear::on_update() {
    if (m_scrollable && m_orientation == orientation::vertical)
        m_scroll_content_height = 0;

    const bool vertical = (m_orientation == orientation::vertical);
    if (m_scrollable && vertical) {
        const auto mouse = bgui::get_mouse_position();
        const bool inside =
            mouse.x >= processed_x() &&
            mouse.x <= processed_x() + processed_width() &&
            mouse.y >= processed_y() &&
            mouse.y <= processed_y() + processed_height();
        auto& context = bgui::get_context();
        if (inside && context.m_scroll_delta_y != 0.f) {
            const float scale = bgui::get_global_scale();
            m_scroll_offset -= static_cast<int>(std::lround(
                context.m_scroll_delta_y * 40.f * scale));
            context.m_scroll_delta_y = 0.f;
        }
    }

    for(auto& [lay, elems] : m_elements) {
    for (auto& elem : m_elements[lay]) {
        if (elem->is_enabled()) {
            elem->calc_content_size(lay);
        }
    }
    calc_content_size(lay);

    if (m_elements[lay].empty()) {
        if (m_scrollable && vertical && lay == layer::base)
            m_scroll_offset = 0;
        continue;
    }

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

    for (auto& elem : m_elements[lay]) {
        if (!elem->is_enabled())
            continue;

        auto mreq = elem->computed_style.layout.size_mode[main];

        if (mreq == mode::pixel || mreq == mode::wrap_content || mreq == mode::same) {
            int margin_main_start = elem->computed_style.layout.margin[main];
            int margin_main_end = elem->computed_style.layout.margin[main + 2];
            vec2i elem_available = available;
            elem_available[main] -= margin_main_start + margin_main_end;
            elem_available[main] = std::max(0, elem_available[main]);
            elem->process_required_size(elem_available);
            fixed_main += elem->processed_size()[main];
            fixed_main += margin_main_start;
            fixed_main += margin_main_end;
        } else {
            stretch_count++;
        }
    }

    float stretch_size = 0.f;
    if (stretch_count > 0) {
        stretch_size = (available[main] - fixed_main) / stretch_count;
        if (stretch_size < 0.f) stretch_size = 0.f;
    }

    for (auto& elem : m_elements[lay]) {
        if(!elem->is_flex()) {
            elem->on_update();
            continue;
        }
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
                    final_available[axis] = axis == main
                        ? static_cast<int>(stretch_size)
                        : axis_available;
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
    for (auto& elem : m_elements[lay]) {
        if(!elem->is_flex()) {
            elem->on_update();
            continue;
        }
        if(!elem->is_enabled()) continue;
        content_main += elem->computed_style.layout.margin[main];
        content_main += elem->processed_size()[main];
        content_main += elem->computed_style.layout.margin[main + 2];
    }

    content_main += pad_main_start + pad_main_end;

    if (m_scrollable && vertical && lay == layer::base) {
        m_scroll_content_height = content_main;
        const int max_scroll = std::max(0, content_main - processed_height());
        m_scroll_offset = std::clamp(m_scroll_offset, 0, max_scroll);
    }

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

    for (auto& elem : m_elements[lay]) {
        if(!elem->is_flex()) {
            elem->on_update();
            continue;
        }
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
                cursor_main + processed_y() - (m_scrollable ? m_scroll_offset : 0),
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

    if (m_scrollable && vertical) {
        const auto drag = m_scrollbar.is_drag();
        if (drag.y != 0) {
            const int padding_top = computed_style.layout.padding.y;
            const int padding_bottom = computed_style.layout.padding.w;
            const int track_height = std::max(
                0, processed_height() - padding_top - padding_bottom);
            const int thumb_travel = std::max(
                0, track_height - m_scrollbar.processed_height());
            if (thumb_travel > 0) {
                const int max_scroll = std::max(
                    0, m_scroll_content_height - processed_height());
                m_scroll_offset += static_cast<int>(
                    static_cast<long long>(drag.y) * max_scroll / thumb_travel);
                m_scroll_offset = std::clamp(m_scroll_offset, 0, max_scroll);
            }
            m_scrollbar.set_drag({0, 0});
        }
        update_scrollbar_rect();
    }
}

void linear::set_scrollable(bool enabled) {
    if (m_scrollable == enabled)
        return;

    m_scrollable = enabled;
    if (!enabled) {
        m_scroll_offset = 0;
        m_scroll_content_height = 0;
        m_scrollbar.set_enable(false);
    }
}

void linear::update_scrollbar_rect() {
    if (!m_scrollable || m_orientation != orientation::vertical ||
        m_scroll_content_height <= processed_height() || processed_height() <= 0) {
        m_scrollbar.set_enable(false);
        return;
    }

    const int scale = std::max(1, static_cast<int>(std::lround(bgui::get_global_scale())));
    const int bar_width = std::min(6 * scale, std::max(0, processed_width()));
    const int padding_top = computed_style.layout.padding.y;
    const int padding_bottom = computed_style.layout.padding.w;
    const int track_height = std::max(0, processed_height() - padding_top - padding_bottom);
    if (track_height <= 0 || m_scroll_content_height <= 0) {
        m_scrollbar.set_enable(false);
        return;
    }

    const int max_scroll = m_scroll_content_height - processed_height();
    const int thumb_height = std::clamp(
        static_cast<int>(static_cast<long long>(track_height) * track_height /
                         m_scroll_content_height),
        std::min(20 * scale, track_height),
        track_height
    );
    const int thumb_travel = track_height - thumb_height;
    const int thumb_y = processed_y() + padding_top +
        static_cast<int>(static_cast<long long>(m_scroll_offset) * thumb_travel / max_scroll);

    m_scrollbar.set_final_rect(
        processed_x() + processed_width() - bar_width - scale,
        thumb_y,
        bar_width,
        thumb_height
    );
    m_scrollbar.set_enable(bar_width > 0 && thumb_height > 0);
}

void linear::get_requires(bgui::draw_data* data) {
    layout::get_requires(data);
    if (!m_scrollbar.is_enabled())
        return;

    const auto inherited_clip = data->m_clip_rect;
    data->m_clip_rect = bgui::intersect_rect(inherited_clip, get_children_clip_rect());
    m_scrollbar.get_requires(data);
    data->m_clip_rect = inherited_clip;
}

void linear::calc_content_size(const layer& lay) {
    const bool vertical = (m_orientation == orientation::vertical);

    int content_w = 0;
    int content_h = 0;

    // Padding
    const int pad_left   = computed_style.layout.padding.x;
    const int pad_top    = computed_style.layout.padding.y;
    const int pad_right  = computed_style.layout.padding.z;
    const int pad_bottom = computed_style.layout.padding.w;

    if (m_elements[lay].empty()) {
        m_content_size = {
            pad_left + pad_right,
            pad_top + pad_bottom
        };
        return;
    }

    for (auto& elem : m_elements[lay]) {
        if (!elem->is_enabled())
            continue;

        const auto& margin = elem->computed_style.layout.margin;

        const auto content_size = elem->get_content_size();
        const int elem_w =
            (elem->computed_style.layout.size_mode[0] == mode::wrap_content ||
             elem->computed_style.layout.size_mode[0] == mode::stretch
                ? content_size.x
                : elem->processed_width()) +
            margin.x + margin.z;

        const int elem_h =
            (elem->computed_style.layout.size_mode[1] == mode::wrap_content ||
             elem->computed_style.layout.size_mode[1] == mode::stretch
                ? content_size.y
                : elem->processed_height()) +
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