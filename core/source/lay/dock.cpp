#include "lay/dock.hpp"

#include "bgui.hpp"
#include "elem/window.hpp"
#include "os/os.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <utility>
#include <unordered_set>

namespace bgui {
    class dock_splitter final : public element {
    public:
        dock_splitter(const bool resize_width, std::function<void(int)> resize)
            : m_resize_width(resize_width), m_resize(std::move(resize)) {
            type = "element";
            add_class("dock-splitter");
            set_flex(false);
            style.layout.limit_min = {0, 0};
        }

        void set_bounds(const vec4i& bounds) {
            set_final_rect(bounds.x, bounds.y, bounds.z, bounds.w);
        }

        void on_update() override {
            const auto delta = is_drag();
            const int amount = m_resize_width ? delta.x : delta.y;
            if (amount == 0)
                return;

            m_resize(amount);
            set_drag({0, 0});
        }

        void on_mouse_hover() override {
            element::on_mouse_hover();
            update_cursor();
        }

        void on_pressed() override {
            element::on_pressed();
            update_cursor();
        }

    private:
        bool m_resize_width;
        std::function<void(int)> m_resize;

        void update_cursor() const {
            get_context().m_actual_cursor = m_resize_width
                ? cursor::resize_horizontal
                : cursor::resize_vertical;
        }
    };

    namespace {
        constexpr std::size_t panel_index(const dock_area area) {
            return static_cast<std::size_t>(area);
        }

        bool is_horizontal_panel(const dock_area area) {
            return area == dock_area::top || area == dock_area::bottom;
        }

        bool has_windows(const std::vector<window*>& windows) {
            return !windows.empty();
        }
    }

    dock::dock() {
        type = "dock";
        style.layout.require_mode(mode::match_parent, mode::match_parent);
        style.layout.padding = vec4i{0};

        m_area_splitters[0] = &add_persistent<dock_splitter, layer::base>(
            true, [this](const int delta) { resize_area(dock_area::left, delta); });
        m_area_splitters[1] = &add_persistent<dock_splitter, layer::base>(
            true, [this](const int delta) { resize_area(dock_area::right, delta); });
        m_area_splitters[2] = &add_persistent<dock_splitter, layer::base>(
            false, [this](const int delta) { resize_area(dock_area::top, delta); });
        m_area_splitters[3] = &add_persistent<dock_splitter, layer::base>(
            false, [this](const int delta) { resize_area(dock_area::bottom, delta); });
    }

    dock::panel& dock::get_panel(const dock_area area) {
        return m_panels[panel_index(area)];
    }

    const dock::panel& dock::get_panel(const dock_area area) const {
        return m_panels[panel_index(area)];
    }

    window& dock::add_window(const std::string& title, const dock_area area) {
        auto& value = add_persistent<window>(title.c_str(), false);
        register_window(value, area);
        return value;
    }

    void dock::register_window(window& value, const dock_area area) {
        for (auto& panel : m_panels) {
            const auto found = std::find(panel.windows.begin(), panel.windows.end(), &value);
            if (found == panel.windows.end())
                continue;

            const auto index = static_cast<std::size_t>(std::distance(panel.windows.begin(), found));
            panel.windows.erase(found);
            if (index < panel.weights.size())
                panel.weights.erase(panel.weights.begin() + static_cast<std::ptrdiff_t>(index));
        }

        auto& panel = get_panel(area);
        const float weight = 1.f / static_cast<float>(panel.windows.size() + 1);
        const float old_total = std::accumulate(panel.weights.begin(), panel.weights.end(), 0.f);
        if (old_total > 0.f) {
            for (auto& old_weight : panel.weights)
                old_weight = old_weight / old_total * (1.f - weight);
        }
        panel.windows.push_back(&value);
        panel.weights.push_back(weight);
    }

    bool dock::remove_window(window* value) {
        if (m_focused_window == value)
            m_focused_window = nullptr;

        bool was_registered = false;
        for (auto& panel : m_panels) {
            for (std::size_t index = 0; index < panel.windows.size();) {
                if (panel.windows[index] != value) {
                    ++index;
                    continue;
                }
                was_registered = true;
                panel.windows.erase(panel.windows.begin() + static_cast<std::ptrdiff_t>(index));
                if (index < panel.weights.size())
                    panel.weights.erase(panel.weights.begin() + static_cast<std::ptrdiff_t>(index));
            }
        }

        if (value && value->get_parent() == this)
            return layout::remove(value) || was_registered;
        return was_registered;
    }

    void dock::sync_windows() {
        std::unordered_set<window*> owned_windows;
        for (auto& [lay, elements] : get_elements()) {
            for (auto& element : elements) {
                if (auto* value = dynamic_cast<window*>(element.get()))
                    owned_windows.insert(value);
            }
        }

        for (auto& panel : m_panels) {
            for (std::size_t index = 0; index < panel.windows.size();) {
                auto* value = panel.windows[index];
                if (owned_windows.contains(value) && !value->is_floating()) {
                    ++index;
                    continue;
                }
                panel.windows.erase(panel.windows.begin() + static_cast<std::ptrdiff_t>(index));
                if (index < panel.weights.size())
                    panel.weights.erase(panel.weights.begin() + static_cast<std::ptrdiff_t>(index));
            }
        }

        for (auto* value : owned_windows) {
            if (value->is_floating())
                continue;
            const bool registered = std::any_of(m_panels.begin(), m_panels.end(), [value](const panel& entry) {
                return std::find(entry.windows.begin(), entry.windows.end(), value) != entry.windows.end();
            });
            if (!registered)
                register_window(*value, dock_area::center);
        }

        if (m_focused_window && !owned_windows.contains(m_focused_window))
            m_focused_window = nullptr;
    }

    void dock::sync_panel_splitters(const dock_area area, panel& value, const std::size_t count) {
        while (value.splitters.size() > count) {
            remove(value.splitters.back());
            value.splitters.pop_back();
        }
        while (value.splitters.size() < count) {
            const std::size_t split_index = value.splitters.size();
            auto& splitter = add_persistent<dock_splitter, layer::base>(
                is_horizontal_panel(area),
                [this, area, split_index](const int delta) {
                    resize_panel_split(area, split_index, delta);
                });
            value.splitters.push_back(&splitter);
        }
    }

    void dock::resize_area(const dock_area area, const int delta) {
        if (area == dock_area::left && m_inner_width > 0)
            m_left_ratio = std::clamp(m_left_ratio + static_cast<float>(delta) / m_inner_width, 0.1f, 0.65f);
        else if (area == dock_area::right && m_inner_width > 0)
            m_right_ratio = std::clamp(m_right_ratio - static_cast<float>(delta) / m_inner_width, 0.1f, 0.65f);
        else if (area == dock_area::top && m_inner_height > 0)
            m_top_ratio = std::clamp(m_top_ratio + static_cast<float>(delta) / m_inner_height, 0.1f, 0.65f);
        else if (area == dock_area::bottom && m_inner_height > 0)
            m_bottom_ratio = std::clamp(m_bottom_ratio - static_cast<float>(delta) / m_inner_height, 0.1f, 0.65f);
    }

    void dock::resize_panel_split(const dock_area area, const std::size_t index, const int delta) {
        auto& panel = get_panel(area);
        if (panel.split_extent <= 0 || index + 1 >= panel.weights.size())
            return;

        const float pair_weight = panel.weights[index] + panel.weights[index + 1];
        const float requested = panel.weights[index] + static_cast<float>(delta) / panel.split_extent;
        const float min_weight = std::min(0.5f * pair_weight, 80.f / panel.split_extent);
        panel.weights[index] = std::clamp(requested, min_weight, pair_weight - min_weight);
        panel.weights[index + 1] = pair_weight - panel.weights[index];
    }

    void dock::focus_window(window* value) {
        if (value && value->get_parent() == this)
            m_focused_window = value;
    }

    dock::configuration dock::get_configuration() {
        configuration result;
        result.left_ratio = m_left_ratio;
        result.right_ratio = m_right_ratio;
        result.top_ratio = m_top_ratio;
        result.bottom_ratio = m_bottom_ratio;

        for (std::size_t panel_index = 0; panel_index < m_panels.size(); ++panel_index) {
            const auto& panel = m_panels[panel_index];
            for (std::size_t window_index = 0; window_index < panel.windows.size(); ++window_index) {
                auto* value = panel.windows[window_index];
                window_configuration entry;
                entry.title = value->get_title().get_buffer();
                entry.area = static_cast<dock_area>(panel_index);
                if (window_index < panel.weights.size())
                    entry.weight = panel.weights[window_index];
                entry.floating = value->is_floating();
                entry.rect = value->processed_rect();
                result.windows.push_back(std::move(entry));
            }
        }

        for (const auto& [layer, elements] : get_elements()) {
            for (const auto& element : elements) {
                auto* value = dynamic_cast<window*>(element.get());
                if (!value || !value->is_floating())
                    continue;
                result.windows.push_back({
                    value->get_title().get_buffer(), dock_area::center, 1.f, true,
                    value->processed_rect()
                });
            }
        }
        return result;
    }

    void dock::apply_configuration(const configuration& value) {
        const auto valid_ratio = [](const float ratio, const float fallback) {
            return std::isfinite(ratio) ? std::clamp(ratio, 0.1f, 0.65f) : fallback;
        };
        m_left_ratio = valid_ratio(value.left_ratio, m_left_ratio);
        m_right_ratio = valid_ratio(value.right_ratio, m_right_ratio);
        m_top_ratio = valid_ratio(value.top_ratio, m_top_ratio);
        m_bottom_ratio = valid_ratio(value.bottom_ratio, m_bottom_ratio);

        for (const auto& entry : value.windows) {
            if (static_cast<std::size_t>(entry.area) >= m_panels.size())
                continue;
            window* target = nullptr;
            for (auto& [layer, elements] : get_elements()) {
                for (auto& element : elements) {
                    auto* candidate = dynamic_cast<window*>(element.get());
                    if (candidate && candidate->get_title().get_buffer() == entry.title) {
                        target = candidate;
                        break;
                    }
                }
                if (target)
                    break;
            }
            if (!target)
                continue;

            target->set_floating(entry.floating);
            if (entry.floating) {
                target->set_position(entry.rect.x, entry.rect.y);
                const float scale = get_global_scale();
                if (scale > 0.f && entry.rect.z > 0 && entry.rect.w > 0) {
                    target->set_final_size(entry.rect.z, entry.rect.w);
                    target->style.layout.require_mode(mode::pixel, mode::pixel);
                    target->style.layout.require_size(entry.rect.z / scale, entry.rect.w / scale);
                    target->mark_style_dirty();
                }
            } else {
                register_window(*target, entry.area);
            }
        }

        for (const auto& entry : value.windows) {
            if (entry.floating || !std::isfinite(entry.weight) || entry.weight <= 0.f)
                continue;
            if (static_cast<std::size_t>(entry.area) >= m_panels.size())
                continue;
            auto& panel = get_panel(entry.area);
            for (std::size_t index = 0; index < panel.windows.size(); ++index) {
                if (panel.windows[index]->get_title().get_buffer() == entry.title) {
                    panel.weights[index] = entry.weight;
                    break;
                }
            }
        }
    }

    void dock::on_update() {
        sync_windows();

        const auto padding = computed_style.layout.padding;
        const int scale = std::max(1, static_cast<int>(std::round(get_global_scale())));
        m_splitter_size = std::max(4, 6 * scale);
        const int min_panel_size = 80 * scale;
        const int x = processed_x() + padding.x;
        const int y = processed_y() + padding.y;
        const int width = std::max(0, processed_width() - padding.x - padding.z);
        const int height = std::max(0, processed_height() - padding.y - padding.w);
        m_inner_width = width;
        m_inner_height = height;

        const bool has_left = has_windows(get_panel(dock_area::left).windows);
        const bool has_right = has_windows(get_panel(dock_area::right).windows);
        const bool has_top = has_windows(get_panel(dock_area::top).windows);
        const bool has_bottom = has_windows(get_panel(dock_area::bottom).windows);
        const bool has_center = has_windows(get_panel(dock_area::center).windows);
        const bool has_middle_column = has_center || has_top || has_bottom;

        const std::array<bool, 3> columns{has_left, has_middle_column, has_right};
        const std::array<float, 3> column_weights{
            m_left_ratio,
            std::max(0.1f, 1.f - m_left_ratio - m_right_ratio),
            m_right_ratio
        };
        const int active_columns = static_cast<int>(has_left) + static_cast<int>(has_middle_column) + static_cast<int>(has_right);
        int column_space = std::max(0, width - std::max(0, active_columns - 1) * m_splitter_size);
        std::array<int, 3> column_sizes{};
        float remaining_column_weight = 0.f;
        for (std::size_t i = 0; i < columns.size(); ++i) {
            if (columns[i])
                remaining_column_weight += column_weights[i];
        }
        int remaining_column_count = active_columns;
        for (std::size_t i = 0; i < columns.size(); ++i) {
            if (!columns[i])
                continue;
            const int desired = remaining_column_count == 1
                ? column_space
                : static_cast<int>(std::round(column_space * column_weights[i] / remaining_column_weight));
            const int min_size = std::min(min_panel_size, column_space / remaining_column_count);
            const int max_size = std::max(min_size, column_space - min_panel_size * (remaining_column_count - 1));
            column_sizes[i] = std::clamp(desired, min_size, max_size);
            column_space -= column_sizes[i];
            remaining_column_weight -= column_weights[i];
            --remaining_column_count;
        }

        int left_x = x;
        int middle_x = x + (has_left ? column_sizes[0] + m_splitter_size : 0);
        int right_x = x + width - column_sizes[2];
        const int middle_width = has_middle_column
            ? column_sizes[1]
            : (has_left && has_right ? 0 : width - column_sizes[0] - column_sizes[2]);
        const int left_width = has_left ? column_sizes[0] : 0;
        const int right_width = has_right ? column_sizes[2] : 0;

        const std::array<bool, 3> rows{has_top, has_center, has_bottom};
        const std::array<float, 3> row_weights{
            m_top_ratio,
            std::max(0.1f, 1.f - m_top_ratio - m_bottom_ratio),
            m_bottom_ratio
        };
        const int active_rows = static_cast<int>(has_top) + static_cast<int>(has_center) + static_cast<int>(has_bottom);
        int row_space = std::max(0, height - std::max(0, active_rows - 1) * m_splitter_size);
        std::array<int, 3> row_sizes{};
        float remaining_row_weight = 0.f;
        for (std::size_t i = 0; i < rows.size(); ++i) {
            if (rows[i])
                remaining_row_weight += row_weights[i];
        }
        int remaining_row_count = active_rows;
        for (std::size_t i = 0; i < rows.size(); ++i) {
            if (!rows[i])
                continue;
            const int desired = remaining_row_count == 1
                ? row_space
                : static_cast<int>(std::round(row_space * row_weights[i] / remaining_row_weight));
            const int min_size = std::min(min_panel_size, row_space / remaining_row_count);
            const int max_size = std::max(min_size, row_space - min_panel_size * (remaining_row_count - 1));
            row_sizes[i] = std::clamp(desired, min_size, max_size);
            row_space -= row_sizes[i];
            remaining_row_weight -= row_weights[i];
            --remaining_row_count;
        }

        int top_y = y;
        int center_y = y + (has_top ? row_sizes[0] + m_splitter_size : 0);
        int bottom_y = y + height - row_sizes[2];
        const int center_height = has_center ? row_sizes[1] : 0;
        const int top_height = has_top ? row_sizes[0] : 0;
        const int bottom_height = has_bottom ? row_sizes[2] : 0;

        get_panel(dock_area::left).bounds = {left_x, y, left_width, height};
        get_panel(dock_area::right).bounds = {right_x, y, right_width, height};
        get_panel(dock_area::top).bounds = {middle_x, top_y, middle_width, top_height};
        get_panel(dock_area::bottom).bounds = {middle_x, bottom_y, middle_width, bottom_height};
        get_panel(dock_area::center).bounds = {middle_x, center_y, middle_width, center_height};

        const bool has_left_split = has_left && (has_middle_column || has_right);
        const bool has_right_split = has_right && has_middle_column;
        const bool has_top_split = has_top && (has_center || has_bottom);
        const bool has_bottom_split = has_bottom && has_center;
        const int left_split_x = has_left_split
            ? x + left_width + (has_middle_column ? 0 : (width - left_width - right_width - m_splitter_size) / 2)
            : 0;
        const int right_split_x = has_right_split ? middle_x + middle_width : 0;
        const int top_split_y = has_top_split
            ? y + top_height + (has_center ? 0 : (height - top_height - bottom_height - m_splitter_size) / 2)
            : 0;
        const int bottom_split_y = has_bottom_split ? center_y + center_height : 0;
        const std::array<bool, 4> splitter_enabled{has_left_split, has_right_split, has_top_split, has_bottom_split};
        const std::array<vec4i, 4> splitter_bounds{
            vec4i{left_split_x, y, m_splitter_size, height},
            vec4i{right_split_x, y, m_splitter_size, height},
            vec4i{middle_x, top_split_y, middle_width, m_splitter_size},
            vec4i{middle_x, bottom_split_y, middle_width, m_splitter_size}
        };
        for (std::size_t i = 0; i < m_area_splitters.size(); ++i) {
            m_area_splitters[i]->set_enable(splitter_enabled[i]);
            m_area_splitters[i]->set_bounds(splitter_bounds[i]);
            if (splitter_enabled[i])
                m_area_splitters[i]->on_update();
        }

        for (std::size_t i = 0; i < m_panels.size(); ++i) {
            const auto area = static_cast<dock_area>(i);
            auto& panel = m_panels[i];
            const auto pinned_count = panel.windows.size();
            sync_panel_splitters(area, panel, pinned_count > 0 ? pinned_count - 1 : 0);

            const bool horizontal = is_horizontal_panel(area);
            panel.split_extent = horizontal ? panel.bounds.z : panel.bounds.w;
            const int total_extent = panel.split_extent;
            const int content_extent = std::max(0, total_extent - m_splitter_size * static_cast<int>(pinned_count > 0 ? pinned_count - 1 : 0));
            float weight_sum = std::accumulate(panel.weights.begin(), panel.weights.end(), 0.f);
            int remaining_extent = content_extent;
            int cursor = horizontal ? panel.bounds.x : panel.bounds.y;
            for (std::size_t window_index = 0; window_index < pinned_count; ++window_index) {
                const int remaining_count = static_cast<int>(pinned_count - window_index);
                const int desired = remaining_count == 1 || weight_sum <= 0.f
                    ? remaining_extent
                    : static_cast<int>(std::round(remaining_extent * panel.weights[window_index] / weight_sum));
                const int min_size = std::min(80 * scale, remaining_extent / remaining_count);
                const int max_size = std::max(min_size, remaining_extent - 80 * scale * (remaining_count - 1));
                const int extent = std::clamp(desired, min_size, max_size);
                auto* value = panel.windows[window_index];
                if (horizontal)
                    value->set_final_rect(cursor, panel.bounds.y, extent, panel.bounds.w);
                else
                    value->set_final_rect(panel.bounds.x, cursor, panel.bounds.z, extent);
                value->on_update();

                if (window_index < panel.splitters.size()) {
                    const vec4i splitter_rect = horizontal
                        ? vec4i{cursor + extent, panel.bounds.y, m_splitter_size, panel.bounds.w}
                        : vec4i{panel.bounds.x, cursor + extent, panel.bounds.z, m_splitter_size};
                    panel.splitters[window_index]->set_enable(true);
                    panel.splitters[window_index]->set_bounds(splitter_rect);
                    panel.splitters[window_index]->on_update();
                }

                cursor += extent + m_splitter_size;
                remaining_extent -= extent;
                weight_sum -= panel.weights[window_index];
            }
        }

        for (auto& [lay, elements] : get_elements()) {
            for (auto& element : elements) {
                if (dynamic_cast<dock_splitter*>(element.get()))
                    continue;
                if (!element->is_enabled())
                    continue;
                if (auto* value = dynamic_cast<window*>(element.get())) {
                    if (value->is_floating()) {
                        value->process_required_size(processed_size());
                        value->on_update();
                    }
                    continue;
                }
                element->process_required_size(processed_size());
                element->on_update();
            }
        }

        auto& base_elements = get_elements()[layer::base];
        std::stable_sort(base_elements.begin(), base_elements.end(), [this](const auto& left, const auto& right) {
            const auto z_order = [this](const auto& element) {
                if (element->has_class("dock-splitter"))
                    return 0;

                const auto* value = dynamic_cast<const window*>(element.get());
                if (!value)
                    return 1;
                if (value->is_floating())
                    return value == m_focused_window ? 5 : 4;
                return value == m_focused_window ? 3 : 2;
            };
            return z_order(left) < z_order(right);
        });
    }
}