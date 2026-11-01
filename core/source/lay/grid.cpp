#include "lay/grid.hpp"
#include "bgui.hpp"

#include <algorithm>
#include <cmath>

using namespace bgui;

namespace {
vec2i scaled(vec2i value) {
    const float scale = bgui::get_global_scale();
    return {static_cast<int>(std::lround(value.x * scale)),
            static_cast<int>(std::lround(value.y * scale))};
}
}

grid::grid(vec2i cell_size, vec2i gap)
    : m_cell_size{std::max(1, cell_size.x), std::max(1, cell_size.y)},
      m_gap{std::max(0, gap.x), std::max(0, gap.y)} {
    type = "grid";
}

int grid::available_width() const {
    int width = processed_width();
    if (auto* parent = get_parent(); parent &&
        (computed_style.layout.size_mode.x == mode::match_parent ||
         computed_style.layout.size_mode.x == mode::stretch)) {
        const auto& padding = parent->computed_style.layout.padding;
        const auto& margin = computed_style.layout.margin;
        width = parent->processed_width() - padding.x - padding.z - margin.x - margin.z;
    }
    const auto& padding = computed_style.layout.padding;
    return std::max(0, width - padding.x - padding.z);
}

int grid::columns(int width) const {
    const auto cell = scaled(m_cell_size);
    const auto gap = scaled(m_gap);
    return std::max(1, (width + gap.x) / std::max(1, cell.x + gap.x));
}

void grid::calc_content_size(const layer& lay) {
    int count = 0;
    for (const auto& elem : m_elements[lay])
        if (elem && elem->is_enabled() && elem->is_flex()) ++count;
    const auto cell = scaled(m_cell_size);
    const auto gap = scaled(m_gap);
    const int cols = columns(available_width());
    const int rows = (count + cols - 1) / cols;
    const int used_cols = std::min(cols, count);
    const auto& padding = computed_style.layout.padding;
    set_content_size({padding.x + padding.z + used_cols * cell.x + std::max(0, used_cols - 1) * gap.x,
                      padding.y + padding.w + rows * cell.y + std::max(0, rows - 1) * gap.y});
}

void grid::arrange() {
    if (!is_layout_dirty()) return;
    clear_layout_dirty();
    const auto& padding = computed_style.layout.padding;
    const int width = std::max(0, processed_width() - padding.x - padding.z);
    const int cols = columns(width);
    const auto cell = scaled(m_cell_size);
    const auto gap = scaled(m_gap);
    const int cell_width = std::min(std::max(1, cell.x), width);
    for (auto& [lay, elems] : m_elements) {
        int index = 0;
        for (auto& elem : elems) {
            if (!elem || !elem->is_enabled()) continue;
            if (!elem->is_flex()) { elem->arrange(); continue; }
            const auto& margin = elem->computed_style.layout.margin;
            elem->process_required_size({std::max(0, cell_width - margin.x - margin.z),
                                         std::max(0, cell.y - margin.y - margin.w)});
            elem->set_final_rect(processed_x() + padding.x + (index % cols) * (cell_width + gap.x) + margin.x,
                                 processed_y() + padding.y + (index / cols) * (cell.y + gap.y) + margin.y,
                                 elem->processed_width(), elem->processed_height());
            elem->arrange();
            ++index;
        }
    }
}
