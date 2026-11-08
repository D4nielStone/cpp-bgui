#include "lay/linear.hpp"
#include "utils/enums.hpp"
#include "bgui.hpp"
#include <algorithm>
#include <cmath>

using namespace bgui;

linear::linear(const orientation& ori) : m_orientation(ori), layout() {
    type = "linear";
    for (auto* bar : {&m_scrollbar, &m_scrollbar_x}) {
        bar->type = "scrollbar";
        bar->set_parent(this);
        bar->receives_input(true);
        bar->set_enable(false);
        bar->style.visual.background.normal = color{0.45f, 0.45f, 0.45f, 0.85f};
        bar->style.visual.border.normal = color{0.f, 0.f, 0.f, 0.f};
        bar->style.visual.border_radius = 3.f;
        bar->style.visual.border_size = 0.f;
        bar->style.visual.visible = true;
    }
}

bool linear::can_scroll_in_direction(const float scroll_delta, const int axis) const {
    if (!m_scrollable || !std::isfinite(scroll_delta) || scroll_delta == 0.f) return false;
    const int content = axis == 0 ? m_scroll_content_width : m_scroll_content_height;
    const int offset = axis == 0 ? m_scroll_offset_x : m_scroll_offset;
    const int max_scroll = std::max(0, content - processed_size()[axis]);
    return scroll_delta < 0.f ? offset < max_scroll : offset > 0;
}

bool linear::has_scrollable_descendant_at(
    const float x,
    const float y,
    const float scroll_delta, const int axis
) {
    auto contains_scrollable = [&](auto&& self, layout& parent) -> bool {
        auto& layers = parent.get_elements();
        for (auto layer_it = layers.rbegin(); layer_it != layers.rend(); ++layer_it) {
            auto& elements = layer_it->second;
            for (auto element_it = elements.rbegin();
                 element_it != elements.rend();
                 ++element_it) {
                auto* child = element_it->get();
                if (!child->is_enabled())
                    continue;

                const auto rect = child->processed_rect();
                if (x < rect.x || x > rect.x + rect.z ||
                    y < rect.y || y > rect.y + rect.w)
                    continue;

                auto* child_layout = child->as_layout();
                if (!child_layout)
                    continue;

                if (child_layout->clips_children()) {
                    const auto clip = child_layout->get_children_clip_rect();
                    if (x < clip.x || x > clip.x + clip.z ||
                        y < clip.y || y > clip.y + clip.w)
                        continue;
                }

                if (auto* scrollable = dynamic_cast<linear*>(child_layout);
                    scrollable &&
                    scrollable->can_scroll_in_direction(scroll_delta, axis))
                    return true;

                if (self(self, *child_layout))
                    return true;
            }
        }
        return false;
    };

    return contains_scrollable(contains_scrollable, *this);
}

void linear::on_update() {
    if (m_scrollable) {
        const auto mouse = bgui::get_mouse_position();
        const auto clip = get_children_clip_rect();
        const bool inside = mouse.x >= clip.x && mouse.x <= clip.x + clip.z &&
            mouse.y >= clip.y && mouse.y <= clip.y + clip.w;
        auto& context = bgui::get_context();
        const bool shift = get_pressed(input_key::left_shift) || get_pressed(input_key::right_shift);
        for (int axis = 0; axis < 2; ++axis) {
            if (axis == 1 && shift) continue;
            float& delta = axis == 0 && (!shift || context.m_scroll_delta_x != 0.f) ? context.m_scroll_delta_x : context.m_scroll_delta_y;
            int& offset = axis == 0 ? m_scroll_offset_x : m_scroll_offset;
            float& remainder = axis == 0 ? m_scroll_remainder_x : m_scroll_remainder;
            const int content = axis == 0 ? m_scroll_content_width : m_scroll_content_height;
            const int max_scroll = std::max(0, content - processed_size()[axis]);
            if (inside && can_scroll_in_direction(delta, axis) &&
                !has_scrollable_descendant_at(mouse.x, mouse.y, delta, axis)) {
                const float pixels = delta * 40.f * bgui::get_global_scale() + remainder;
                const int movement = static_cast<int>(std::clamp(std::round(double(pixels)),
                    -double(max_scroll), double(max_scroll)));
                const int next = std::clamp(offset - movement, 0, max_scroll);
                if (next != offset || movement == 0) {
                    offset = next;
                    remainder = movement == 0 || std::abs(pixels) < max_scroll ? pixels - movement : 0.f;
                    delta = 0.f;
                    mark_layout_dirty();
                }
            }
            auto& bar = axis == 0 ? m_scrollbar_x : m_scrollbar;
            const auto drag = bar.is_drag();
            if (drag[axis] != 0) {
                const int padding = computed_style.layout.padding[axis] + computed_style.layout.padding[axis + 2];
                const int travel = std::max(0, processed_size()[axis] - padding - bar.processed_size()[axis] -
                    ((axis == 0 ? m_scroll_content_height > processed_height() : m_scroll_content_width > processed_width())
                        ? 7 * std::max(1, static_cast<int>(std::lround(bgui::get_global_scale()))) : 0));
                if (travel > 0) {
                    const auto next = static_cast<long long>(offset) + static_cast<long long>(drag[axis]) * max_scroll / travel;
                    offset = static_cast<int>(std::clamp(next, 0LL, static_cast<long long>(max_scroll)));
                    mark_layout_dirty();
                }
                bar.set_drag({0, 0});
            }
        }
    }
    layout::on_update();
}

void linear::arrange() {
    if (!is_layout_dirty())
        return;
    clear_layout_dirty();

    const bool vertical = (m_orientation == orientation::vertical);
    if (m_scrollable) { m_scroll_content_height = 0; m_scroll_content_width = 0; }

    for(auto& [lay, elems] : m_elements) {
    for (auto& elem : m_elements[lay]) {
        if (!elem || !elem->is_enabled() || !elem->is_flex())
            continue;
        elem->calc_content_size(lay);
    }
    calc_content_size(lay);

    if (m_elements[lay].empty()) {
        if (m_scrollable && lay == layer::base) { m_scroll_offset = 0; m_scroll_offset_x = 0; }
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
    int flexible_margins = 0;
    int stretch_count = 0;

    for (auto& elem : m_elements[lay]) {
        if (!elem || !elem->is_enabled() || !elem->is_flex())
            continue;

        auto mreq = elem->computed_style.layout.size_mode[main];
        const int margin_main_start = elem->computed_style.layout.margin[main];
        const int margin_main_end = elem->computed_style.layout.margin[main + 2];

        if (mreq == mode::stretch || mreq == mode::match_parent) {
            flexible_margins += margin_main_start + margin_main_end;
            ++stretch_count;
        } else {
            vec2i elem_available = available;
            elem_available[main] -= margin_main_start + margin_main_end;
            elem_available[main] = std::max(0, elem_available[main]);
            elem->process_required_size(elem_available);
            fixed_main += elem->processed_size()[main];
            fixed_main += margin_main_start + margin_main_end;
        }
    }

    float stretch_size = 0.f;
    if (stretch_count > 0) {
        stretch_size =
            (available[main] - fixed_main - flexible_margins) / stretch_count;
        if (stretch_size < 0.f) stretch_size = 0.f;
    }

    for (auto& elem : m_elements[lay]) {
        if (!elem || !elem->is_enabled() || !elem->is_flex())
            continue;
        vec2i final_available = elem->processed_size();
        if (elem->computed_style.layout.size_mode[main] == mode::percent) {
            const int margin_before = elem->computed_style.layout.margin[main];
            const int margin_after = elem->computed_style.layout.margin[main + 2];
            final_available[main] = std::max(
                0, available[main] - margin_before - margin_after);
        }

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
                    } else {
                        final_available[axis] = axis_available;
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
        if (!elem || !elem->is_enabled() || !elem->is_flex())
            continue;
        content_main += elem->computed_style.layout.margin[main];
        content_main += elem->processed_size()[main];
        content_main += elem->computed_style.layout.margin[main + 2];
    }

    content_main += pad_main_start + pad_main_end;

    if (m_scrollable && lay == layer::base) {
        int content_cross = pad_cross_start + pad_cross_end;
        for (auto& elem : m_elements[lay]) if (elem && elem->is_enabled() && elem->is_flex())
            content_cross = std::max(content_cross, pad_cross_start + pad_cross_end + elem->processed_size()[cross] +
                elem->computed_style.layout.margin[cross] + elem->computed_style.layout.margin[cross + 2]);
        m_scroll_content_height = vertical ? content_main : content_cross;
        m_scroll_content_width = vertical ? content_cross : content_main;
        m_scroll_offset = std::clamp(m_scroll_offset, 0, std::max(0, m_scroll_content_height - processed_height()));
        m_scroll_offset_x = std::clamp(m_scroll_offset_x, 0, std::max(0, m_scroll_content_width - processed_width()));
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
        if (!elem || !elem->is_enabled() || !elem->is_flex())
            continue;

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
                cross_pos + processed_x() - (m_scrollable ? m_scroll_offset_x : 0),
                cursor_main + processed_y() - (m_scrollable ? m_scroll_offset : 0),
                elem->processed_width(),
                elem->processed_height()
            );
        } else {
            elem->set_final_rect(
                cursor_main + processed_x() - (m_scrollable ? m_scroll_offset_x : 0),
                cross_pos + processed_y() - (m_scrollable ? m_scroll_offset : 0),
                elem->processed_width(),
                elem->processed_height()
            );
        }

        cursor_main += elem->processed_size()[main];
        cursor_main += elem->computed_style.layout.margin[main + 2];
        elem->arrange();
    }

    for (auto& elem : m_elements[lay]) {
        if (elem && elem->is_enabled() && !elem->is_flex())
            elem->arrange();
    }
    }

    if (m_scrollable) update_scrollbar_rect();
}

void linear::prepare_geometry_frame() {
    layout::prepare_geometry_frame();
    m_scrollbar.prepare_geometry_frame();
    m_scrollbar_x.prepare_geometry_frame();
}

void linear::commit_geometry() {
    layout::commit_geometry();
    m_scrollbar.commit_geometry();
    m_scrollbar_x.commit_geometry();
}

void linear::set_scrollable(bool enabled) {
    if (m_scrollable == enabled)
        return;

    m_scrollable = enabled;
    mark_layout_dirty();
    if (!enabled) {
        m_scroll_offset_x = m_scroll_content_width = 0;
        m_scroll_remainder_x = 0.f;
        m_scrollbar_x.set_enable(false);
        m_scroll_offset = 0;
        m_scroll_content_height = 0;
        m_scroll_remainder = 0.f;
        m_scrollbar.set_enable(false);
    }
}

void linear::update_scrollbar_rect() {
    const int scale = std::max(1, static_cast<int>(std::lround(bgui::get_global_scale())));
    for (int axis = 0; axis < 2; ++axis) {
        auto& bar = axis == 0 ? m_scrollbar_x : m_scrollbar;
        const int extent = processed_size()[axis];
        const int content = axis == 0 ? m_scroll_content_width : m_scroll_content_height;
        const int offset = axis == 0 ? m_scroll_offset_x : m_scroll_offset;
        const int padding_start = computed_style.layout.padding[axis];
        const int padding_end = computed_style.layout.padding[axis + 2];
        const int track = std::max(0, extent - padding_start - padding_end -
            ((axis == 0 ? m_scroll_content_height > processed_height() : m_scroll_content_width > processed_width()) ? 7 * scale : 0));
        const int thickness = std::min(6 * scale, std::max(0, processed_size()[1 - axis]));
        if (!m_scrollable || content <= extent || track <= 0 || thickness <= 0) { bar.set_enable(false); continue; }
        const int thumb = std::clamp(static_cast<int>(static_cast<long long>(track) * track / content), std::min(20 * scale, track), track);
        const int position = static_cast<int>(static_cast<long long>(offset) * (track - thumb) / (content - extent));
        if (axis == 0) bar.set_final_rect(processed_x() + padding_start + position,
            processed_y() + processed_height() - thickness - scale, thumb, thickness);
        else bar.set_final_rect(processed_x() + processed_width() - thickness - scale,
            processed_y() + padding_start + position, thickness, thumb);
        bar.set_enable(true);
    }
}

void linear::get_requires(bgui::draw_data* data) {
    layout::get_requires(data);
    const auto inherited_clip = data->m_clip_rect;
    data->m_clip_rect = bgui::intersect_rect(inherited_clip, get_children_clip_rect());
    for (auto* bar : {&m_scrollbar, &m_scrollbar_x}) if (bar->is_enabled()) bar->get_cached_requires(data);
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
        if (!elem || !elem->is_enabled() || !elem->is_flex())
            continue;

        const auto& margin = elem->computed_style.layout.margin;

        const auto content_size = elem->get_content_size();
        const auto measure_axis = [&](const int axis) {
            const auto& layout = elem->computed_style.layout;
            if (layout.size_mode[axis] == mode::wrap_content ||
                layout.size_mode[axis] == mode::stretch ||
                (layout.size_mode[axis] == mode::match_parent &&
                 computed_style.layout.size_mode[axis] == mode::wrap_content))
                return std::clamp(content_size[axis],
                    layout.limit_min[axis], layout.limit_max[axis]);
            if (layout.size_mode[axis] == mode::pixel) {
                return std::clamp(
                    static_cast<int>(layout.size[axis]),
                    layout.limit_min[axis],
                    layout.limit_max[axis]
                );
            }
            return elem->processed_size()[axis];
        };
        const int elem_w = measure_axis(0) + margin.x + margin.z;
        const int elem_h = measure_axis(1) + margin.y + margin.w;

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