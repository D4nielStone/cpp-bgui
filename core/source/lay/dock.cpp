#include "lay/dock.hpp"

#include "bgui.hpp"
#include "elem/button.hpp"
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
            style.visual.visible = false;
            mark_style_dirty();
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

        constexpr std::array<const char*, 5> names{ "left", "right", "top", "bottom", "center" };
        for (std::size_t index = 0; index < m_drop_targets.size(); ++index) {
            auto& target = add_persistent<element, layer::base>();
            target.recives_input(false);
            target.set_enable(false);
            target.style.visual.background.normal = bgui::color{0.15f, 0.55f, 1.f, 0.45f};
            target.style.visual.border.normal = bgui::color{0.25f, 0.75f, 1.f, 1.f};
            target.style.visual.border_size = 2.f;
            target.style.visual.border_radius = 6.f;
            target.style.visual.visible = false;
            target.mark_style_dirty();
            target.add_class("dock-drop-zone");
            target.add_class(std::string("dock-drop-zone-") + names[index]);
            m_drop_targets[index] = &target;
        }

        auto& preview = add_persistent<element, layer::base>();
        preview.recives_input(false);
        preview.set_enable(false);
        preview.style.visual.background.normal = bgui::color{0.15f, 0.55f, 1.f, 0.32f};
        preview.style.visual.border.normal = bgui::color{0.25f, 0.75f, 1.f, 1.f};
        preview.style.visual.border_size = 3.f;
        preview.style.visual.visible = false;
        preview.mark_style_dirty();
        preview.add_class("dock-drop-preview");
        m_drop_preview = &preview;
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
        const auto is_panel_root = [this](const window* candidate) {
            const bool split_child = std::any_of(m_nested_splits.begin(), m_nested_splits.end(), [candidate](const nested_split& split) {
                return split.added == candidate;
            });
            const bool tab_child = std::any_of(m_tab_groups.begin(), m_tab_groups.end(), [candidate](const tab_group& group) {
                return group.anchor != candidate &&
                    std::find(group.windows.begin(), group.windows.end(), candidate) != group.windows.end();
            });
            return !split_child && !tab_child;
        };
        std::size_t root_count = 0;
        float old_total = 0.f;
        for (std::size_t index = 0; index < panel.windows.size(); ++index) {
            if (!is_panel_root(panel.windows[index]))
                continue;
            ++root_count;
            if (index < panel.weights.size())
                old_total += panel.weights[index];
        }
        const float weight = 1.f / static_cast<float>(root_count + 1);
        if (old_total > 0.f) {
            for (std::size_t index = 0; index < panel.windows.size() && index < panel.weights.size(); ++index) {
                if (is_panel_root(panel.windows[index]))
                    panel.weights[index] = panel.weights[index] / old_total * (1.f - weight);
            }
        }
        panel.windows.push_back(&value);
        panel.weights.push_back(weight);
    }

    void dock::split_window(window& anchor, window& added, const bool horizontal, const bool after) {
        dock_area area = dock_area::center;
        for (std::size_t index = 0; index < m_panels.size(); ++index) {
            const auto& windows = m_panels[index].windows;
            if (std::find(windows.begin(), windows.end(), &anchor) != windows.end()) {
                area = static_cast<dock_area>(index);
                break;
            }
        }
        register_window(added, area);
        m_nested_splits.push_back({&anchor, &added, horizontal, after});
        auto& split = m_nested_splits.back();
        split.splitter = &add_persistent<dock_splitter, layer::base>(
            horizontal, [this, anchor_ptr = &anchor, added_ptr = &added](const int delta) {
                const auto found = std::find_if(m_nested_splits.begin(), m_nested_splits.end(),
                    [anchor_ptr, added_ptr](const nested_split& candidate) {
                        return candidate.anchor == anchor_ptr && candidate.added == added_ptr;
                    });
                if (found == m_nested_splits.end() || found->extent <= 0)
                    return;
                const float minimum = std::min(0.5f, 80.f / found->extent);
                found->ratio = std::clamp(
                    found->ratio + static_cast<float>(delta) / found->extent,
                    minimum,
                    1.f - minimum
                );
            });
    }

    void dock::merge_window_as_tab(window& anchor, window& added) {
        auto group = std::find_if(m_tab_groups.begin(), m_tab_groups.end(), [&anchor](const tab_group& candidate) {
            return std::find(candidate.windows.begin(), candidate.windows.end(), &anchor) != candidate.windows.end();
        });
        if (group == m_tab_groups.end()) {
            m_tab_groups.push_back({&anchor, &anchor, {&anchor}, {}});
            group = std::prev(m_tab_groups.end());
        }

        if (std::find(group->windows.begin(), group->windows.end(), &added) != group->windows.end())
            return;

        for (auto* member : group->windows)
            member->set_tabbed(true);

        dock_area area = dock_area::center;
        for (std::size_t index = 0; index < m_panels.size(); ++index) {
            const auto& windows = m_panels[index].windows;
            if (std::find(windows.begin(), windows.end(), &anchor) != windows.end()) {
                area = static_cast<dock_area>(index);
                break;
            }
        }
        register_window(added, area);
        group->windows.push_back(&added);
        group->active = &added;
        added.set_tabbed(true);

        for (std::size_t index = group->buttons.size(); index < group->windows.size(); ++index) {
            auto* tab_window = group->windows[index];
            auto& tab = add_persistent<button, layer::base>(
                tab_window->get_title().get_buffer(), 0.4f,
                [this, host = group->anchor, tab_window]() {
                    const auto found = std::find_if(m_tab_groups.begin(), m_tab_groups.end(), [host](const tab_group& candidate) {
                        return candidate.anchor == host;
                    });
                    if (found != m_tab_groups.end()) {
                        found->active = tab_window;
                        focus_window(tab_window);
                    }
                });
            tab.add_class("dock-tab");
            group->buttons.push_back(&tab);
        }
    }

    bool dock::remove_window(window* value) {
        if (m_focused_window == value)
            m_focused_window = nullptr;
        if (m_dragged_window == value)
            m_dragged_window = nullptr;
        if (m_tab_dragged_window == value)
            m_tab_dragged_window = nullptr;
        if (m_drop_target_window == value)
            m_drop_target_window = nullptr;

        m_nested_splits.erase(std::remove_if(m_nested_splits.begin(), m_nested_splits.end(), [this, value](const nested_split& split) {
            if ((split.added == value || split.anchor == value) && split.splitter)
                remove(split.splitter);
            return split.added == value || split.anchor == value;
        }), m_nested_splits.end());

        for (auto group = m_tab_groups.begin(); group != m_tab_groups.end();) {
            if (std::find(group->windows.begin(), group->windows.end(), value) == group->windows.end()) {
                ++group;
                continue;
            }
            for (auto* member : group->windows) {
                if (member != value) {
                    member->set_enable(true);
                    member->set_tabbed(false);
                }
            }
            for (auto* tab : group->buttons)
                remove(tab);
            group = m_tab_groups.erase(group);
        }

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

        if (m_dragged_window && !owned_windows.contains(m_dragged_window))
            m_dragged_window = nullptr;
        if (m_tab_dragged_window && !owned_windows.contains(m_tab_dragged_window))
            m_tab_dragged_window = nullptr;

        m_nested_splits.erase(std::remove_if(m_nested_splits.begin(), m_nested_splits.end(), [this, &owned_windows](const nested_split& split) {
            const bool remove_split = !owned_windows.contains(split.anchor) || !owned_windows.contains(split.added) ||
                split.anchor->is_floating() || split.added->is_floating();
            if (remove_split && split.splitter)
                remove(split.splitter);
            return remove_split;
        }), m_nested_splits.end());

        for (auto group = m_tab_groups.begin(); group != m_tab_groups.end();) {
            const bool has_floating_member = std::any_of(group->windows.begin(), group->windows.end(), [](const window* value) {
                return value->is_floating();
            });
            if (!has_floating_member) {
                ++group;
                continue;
            }
            for (auto* member : group->windows) {
                member->set_enable(true);
                member->set_tabbed(false);
            }
            for (auto* tab : group->buttons)
                remove(tab);
            group = m_tab_groups.erase(group);
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
        if (panel.split_extent <= 0)
            return;

        std::vector<std::size_t> root_indices;
        for (std::size_t window_index = 0; window_index < panel.windows.size(); ++window_index) {
            auto* value = panel.windows[window_index];
            const bool split_child = std::any_of(m_nested_splits.begin(), m_nested_splits.end(), [value](const nested_split& split) {
                return split.added == value;
            });
            const bool tab_child = std::any_of(m_tab_groups.begin(), m_tab_groups.end(), [value](const tab_group& group) {
                return group.anchor != value &&
                    std::find(group.windows.begin(), group.windows.end(), value) != group.windows.end();
            });
            if (!split_child && !tab_child && window_index < panel.weights.size())
                root_indices.push_back(window_index);
        }
        if (index + 1 >= root_indices.size())
            return;

        const auto first_weight = root_indices[index];
        const auto second_weight = root_indices[index + 1];
        const float pair_weight = panel.weights[first_weight] + panel.weights[second_weight];
        const float requested = panel.weights[first_weight] + static_cast<float>(delta) / panel.split_extent;
        const float min_weight = std::min(0.5f * pair_weight, 80.f / panel.split_extent);
        panel.weights[first_weight] = std::clamp(requested, min_weight, pair_weight - min_weight);
        panel.weights[second_weight] = pair_weight - panel.weights[first_weight];
    }

    void dock::focus_window(window* value) {
        if (value && value->get_parent() == this)
            m_focused_window = value;
    }

    dock::configuration dock::get_configuration() {
        sync_windows();
        configuration result;
        result.left_ratio = m_left_ratio;
        result.right_ratio = m_right_ratio;
        result.top_ratio = m_top_ratio;
        result.bottom_ratio = m_bottom_ratio;

        for (std::size_t panel_index = 0; panel_index < m_panels.size(); ++panel_index) {
            const auto& panel = m_panels[panel_index];
            for (std::size_t window_index = 0; window_index < panel.windows.size(); ++window_index) {
                auto* value = panel.windows[window_index];
                if (value->is_floating())
                    continue;
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

        for (const auto& split : m_nested_splits) {
            if (!split.anchor || !split.added)
                continue;
            result.splits.push_back({
                split.anchor->get_title().get_buffer(),
                split.added->get_title().get_buffer(),
                split.horizontal,
                split.after,
                split.ratio
            });
        }

        for (const auto& group : m_tab_groups) {
            if (!group.anchor)
                continue;
            tab_group_configuration entry;
            entry.anchor = group.anchor->get_title().get_buffer();
            if (group.active)
                entry.active = group.active->get_title().get_buffer();
            for (auto* value : group.windows) {
                if (value)
                    entry.windows.push_back(value->get_title().get_buffer());
            }
            result.tab_groups.push_back(std::move(entry));
        }
        return result;
    }

    void dock::update_drop_targets() {
        const auto pointer = bgui::get_mouse_position();
        const int scale = std::max(1, static_cast<int>(std::round(get_global_scale())));
        const int splitter_size = std::max(4, 6 * scale);
        const auto dragged = std::find_if(get_elements()[layer::base].begin(), get_elements()[layer::base].end(), [](const auto& element) {
            auto* value = dynamic_cast<window*>(element.get());
            if (!value || !value->is_floating())
                return false;
            const auto drag = value->get_title().is_drag();
            return value->is_dragging() || drag.x != 0 || drag.y != 0;
        });
        const bool tab_dragging = m_tab_dragged_window && m_tab_dragged_window->is_floating();
        if (tab_dragging) {
            m_tab_dragged_window->set_position(
                pointer.x - m_tab_drag_offset.x,
                pointer.y - m_tab_drag_offset.y
            );
            m_dragged_window = m_tab_dragged_window;
        }
        const bool has_dragged_window =
            dragged != get_elements()[layer::base].end() || tab_dragging;
        if (has_dragged_window && !tab_dragging)
            m_dragged_window = dynamic_cast<window*>(dragged->get());

        if (m_dragged_window && !bgui::get_pressed(input_key::mouse_left)) {
            for (std::size_t index = 0; index < m_drop_targets.size(); ++index) {
                const auto rect = m_drop_targets[index]->processed_rect();
                const bool pointer_inside = pointer.x >= rect.x && pointer.x <= rect.x + rect.z &&
                    pointer.y >= rect.y && pointer.y <= rect.y + rect.w;
                if (!m_drop_targets[index]->is_enabled() || !pointer_inside)
                    continue;

                m_dragged_window->set_floating(false);
                if (m_drop_target_window) {
                    if (index == panel_index(dock_area::center)) {
                        merge_window_as_tab(*m_drop_target_window, *m_dragged_window);
                    } else {
                        auto* anchor = m_drop_target_window;
                        const auto tab_group = std::find_if(m_tab_groups.begin(), m_tab_groups.end(), [anchor](const dock::tab_group& group) {
                            return std::find(group.windows.begin(), group.windows.end(), anchor) != group.windows.end();
                        });
                        if (tab_group != m_tab_groups.end())
                            anchor = tab_group->anchor;
                        split_window(
                            *anchor,
                            *m_dragged_window,
                            index < 2,
                            index == panel_index(dock_area::right) ||
                                index == panel_index(dock_area::bottom)
                        );
                    }
                } else {
                    register_window(*m_dragged_window, static_cast<dock_area>(index));
                }
                focus_window(m_dragged_window);
                break;
            }
            m_dragged_window = nullptr;
            m_drop_target_window = nullptr;
            m_tab_dragged_window = nullptr;
        }
        if (!bgui::get_pressed(input_key::mouse_left) && m_tab_dragged_window &&
            !m_tab_dragged_window->is_floating())
            m_tab_dragged_window = nullptr;

        const auto padding = computed_style.layout.padding;
        const int x = processed_x() + padding.x;
        const int y = processed_y() + padding.y;
        const int width = std::max(0, processed_width() - padding.x - padding.z);
        const int height = std::max(0, processed_height() - padding.y - padding.w);
        const bool pointer_inside = pointer.x >= x && pointer.x <= x + width && pointer.y >= y && pointer.y <= y + height;

        if (!pointer_inside || !has_dragged_window ||
            !bgui::get_pressed(input_key::mouse_left)) {
            for (auto* target : m_drop_targets) {
                target->set_enable(false);
                target->style.visual.visible = false;
                target->mark_style_dirty();
            }
            m_drop_preview->set_enable(false);
            m_drop_preview->style.visual.visible = false;
            m_drop_preview->mark_style_dirty();
            return;
        }

        m_drop_target_window = nullptr;
        vec4i target_bounds{x, y, width, height};
        for (auto& panel : m_panels) {
            for (auto* value : panel.windows) {
                if (!value->is_enabled() || value->is_floating())
                    continue;
                const auto rect = value->processed_rect();
                if (pointer.x < rect.x || pointer.x > rect.x + rect.z ||
                    pointer.y < rect.y || pointer.y > rect.y + rect.w)
                    continue;
                m_drop_target_window = value;
                target_bounds = rect;
            }
        }
        for (const auto& group : m_tab_groups) {
            if (!group.active || group.buttons.empty())
                continue;
            const auto active_rect = group.active->processed_rect();
            const auto tabs_rect = group.buttons.front()->processed_rect();
            const vec4i group_bounds{
                active_rect.x, tabs_rect.y, active_rect.z, active_rect.w + tabs_rect.w
            };
            if (pointer.x < group_bounds.x || pointer.x > group_bounds.x + group_bounds.z ||
                pointer.y < group_bounds.y || pointer.y > group_bounds.y + group_bounds.w)
                continue;
            m_drop_target_window = group.active;
            target_bounds = group_bounds;
        }

        const int target_width = std::max(0, target_bounds.z);
        const int target_height = std::max(0, target_bounds.w);
        const int target_size = std::clamp(std::min(target_width, target_height) / 4, 30, 72);
        const int horizontal_center = target_bounds.x + target_width / 2;
        const int vertical_center = target_bounds.y + target_height / 2;
        const std::array<vec4i, 5> rects = m_drop_target_window
            ? std::array<vec4i, 5>{
                vec4i{target_bounds.x + target_width / 4 - target_size / 2,
                      vertical_center - target_size / 2, target_size, target_size},
                vec4i{target_bounds.x + target_width * 3 / 4 - target_size / 2,
                      vertical_center - target_size / 2, target_size, target_size},
                vec4i{horizontal_center - target_size / 2,
                      target_bounds.y + target_height / 4 - target_size / 2, target_size, target_size},
                vec4i{horizontal_center - target_size / 2,
                      target_bounds.y + target_height * 3 / 4 - target_size / 2, target_size, target_size},
                vec4i{horizontal_center - target_size / 2, vertical_center - target_size / 2,
                      target_size, target_size}
            }
            : std::array<vec4i, 5>{
                vec4i{x + 18, y + (height - target_size) / 2, target_size, target_size},
                vec4i{x + width - target_size - 18, y + (height - target_size) / 2, target_size, target_size},
                vec4i{x + (width - target_size) / 2, y + 18, target_size, target_size},
                vec4i{x + (width - target_size) / 2, y + height - target_size - 18, target_size, target_size},
                vec4i{x + (width - target_size) / 2, y + (height - target_size) / 2, target_size, target_size}
            };

        for (std::size_t index = 0; index < m_drop_targets.size(); ++index) {
            auto* target = m_drop_targets[index];
            target->set_enable(true);
            target->set_final_rect(rects[index].x, rects[index].y, rects[index].z, rects[index].w);
            target->style.visual.visible = true;
            target->mark_style_dirty();
        }

        std::size_t hovered_target = m_drop_targets.size();
        for (std::size_t index = 0; index < m_drop_targets.size(); ++index) {
            const auto& rect = rects[index];
            if (pointer.x >= rect.x && pointer.x <= rect.x + rect.z &&
                pointer.y >= rect.y && pointer.y <= rect.y + rect.w) {
                hovered_target = index;
                break;
            }
        }
        if (hovered_target == m_drop_targets.size()) {
            m_drop_preview->set_enable(false);
            m_drop_preview->style.visual.visible = false;
            m_drop_preview->mark_style_dirty();
            return;
        }

        vec4i preview_bounds{};
        if (m_drop_target_window) {
            if (hovered_target == panel_index(dock_area::center)) {
                const auto group = std::find_if(m_tab_groups.begin(), m_tab_groups.end(), [this](const tab_group& candidate) {
                    return std::find(candidate.windows.begin(), candidate.windows.end(), m_drop_target_window) != candidate.windows.end();
                });
                if (group != m_tab_groups.end()) {
                    preview_bounds = m_drop_target_window->processed_rect();
                } else {
                    preview_bounds = target_bounds;
                    const int tab_height = std::min(28 * scale, std::max(0, preview_bounds.w / 4));
                    preview_bounds.y += tab_height;
                    preview_bounds.w = std::max(0, preview_bounds.w - tab_height);
                }
            } else if (hovered_target < 2) {
                const int available = std::max(0, target_bounds.z - splitter_size);
                const int preview_width = static_cast<int>(std::round(available * 0.5f));
                preview_bounds = hovered_target == panel_index(dock_area::left)
                    ? vec4i{target_bounds.x, target_bounds.y, preview_width, target_bounds.w}
                    : vec4i{target_bounds.x + preview_width + splitter_size, target_bounds.y,
                            available - preview_width, target_bounds.w};
            } else {
                const int available = std::max(0, target_bounds.w - splitter_size);
                const int preview_height = static_cast<int>(std::round(available * 0.5f));
                preview_bounds = hovered_target == panel_index(dock_area::top)
                    ? vec4i{target_bounds.x, target_bounds.y, target_bounds.z, preview_height}
                    : vec4i{target_bounds.x, target_bounds.y + preview_height + splitter_size,
                            target_bounds.z, available - preview_height};
            }
        } else if (hovered_target == panel_index(dock_area::left)) {
            const int preview_width = static_cast<int>(width * m_left_ratio);
            preview_bounds = {x, y, preview_width, height};
        } else if (hovered_target == panel_index(dock_area::right)) {
            const int preview_width = static_cast<int>(width * m_right_ratio);
            preview_bounds = {x + width - preview_width, y, preview_width, height};
        } else if (hovered_target == panel_index(dock_area::top)) {
            const int preview_height = static_cast<int>(height * m_top_ratio);
            preview_bounds = {x, y, width, preview_height};
        } else if (hovered_target == panel_index(dock_area::bottom)) {
            const int preview_height = static_cast<int>(height * m_bottom_ratio);
            preview_bounds = {x, y + height - preview_height, width, preview_height};
        } else {
            preview_bounds = get_panel(dock_area::center).bounds;
            if (preview_bounds.z <= 0 || preview_bounds.w <= 0)
                preview_bounds = {x, y, width, height};
        }

        if (!m_drop_target_window) {
            auto& panel = get_panel(static_cast<dock_area>(hovered_target));
            const auto panel_bounds = panel.bounds;
            if (panel_bounds.z > 0 && panel_bounds.w > 0) {
                std::size_t root_count = 0;
                for (auto* value : panel.windows) {
                    const bool split_child = std::any_of(m_nested_splits.begin(), m_nested_splits.end(), [value](const nested_split& split) {
                        return split.added == value;
                    });
                    const bool tab_child = std::any_of(m_tab_groups.begin(), m_tab_groups.end(), [value](const tab_group& group) {
                        return group.anchor != value &&
                            std::find(group.windows.begin(), group.windows.end(), value) != group.windows.end();
                    });
                    if (!split_child && !tab_child)
                        ++root_count;
                }

                const bool horizontal = is_horizontal_panel(static_cast<dock_area>(hovered_target));
                const int extent = horizontal ? panel_bounds.z : panel_bounds.w;
                const int content_extent = std::max(
                    0, extent - splitter_size * static_cast<int>(root_count)
                );
                const int preview_extent = std::min(
                    content_extent,
                    static_cast<int>(std::round(content_extent / static_cast<float>(root_count + 1)))
                );
                if (horizontal) {
                    preview_bounds = {
                        panel_bounds.x + panel_bounds.z - preview_extent,
                        panel_bounds.y, preview_extent, panel_bounds.w
                    };
                } else {
                    preview_bounds = {
                        panel_bounds.x, panel_bounds.y + panel_bounds.w - preview_extent,
                        panel_bounds.z, preview_extent
                    };
                }
            }
        }

        m_drop_preview->set_enable(true);
        m_drop_preview->set_final_rect(
            preview_bounds.x, preview_bounds.y, preview_bounds.z, preview_bounds.w
        );
        m_drop_preview->style.visual.visible = true;
        m_drop_preview->mark_style_dirty();
    }

    void dock::apply_configuration(const configuration& value) {
        const auto valid_ratio = [](const float ratio, const float fallback) {
            return std::isfinite(ratio) ? std::clamp(ratio, 0.1f, 0.65f) : fallback;
        };
        m_left_ratio = valid_ratio(value.left_ratio, m_left_ratio);
        m_right_ratio = valid_ratio(value.right_ratio, m_right_ratio);
        m_top_ratio = valid_ratio(value.top_ratio, m_top_ratio);
        m_bottom_ratio = valid_ratio(value.bottom_ratio, m_bottom_ratio);

        for (const auto& split : m_nested_splits) {
            if (split.splitter)
                remove(split.splitter);
        }
        for (const auto& group : m_tab_groups) {
            for (auto* member : group.windows) {
                if (member) {
                    member->set_enable(true);
                    member->set_tabbed(false);
                }
            }
            for (auto* tab : group.buttons)
                remove(tab);
        }
        m_nested_splits.clear();
        m_tab_groups.clear();
        m_tab_dragged_window = nullptr;
        for (auto& panel : m_panels) {
            panel.windows.clear();
            panel.weights.clear();
        }

        const auto find_window = [this](const std::string& title) -> window* {
            for (auto& [layer, elements] : get_elements()) {
                for (auto& element : elements) {
                    auto* candidate = dynamic_cast<window*>(element.get());
                    if (candidate && candidate->get_title().get_buffer() == title)
                        return candidate;
                }
            }
            return nullptr;
        };

        for (const auto& entry : value.windows) {
            if (static_cast<std::size_t>(entry.area) >= m_panels.size())
                continue;
            auto* target = find_window(entry.title);
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

        for (const auto& entry : value.splits) {
            auto* anchor = find_window(entry.anchor);
            auto* added = find_window(entry.added);
            if (!anchor || !added || anchor == added || anchor->is_floating() || added->is_floating())
                continue;
            split_window(*anchor, *added, entry.horizontal, entry.after);
            auto& split = m_nested_splits.back();
            split.ratio = std::isfinite(entry.ratio)
                ? std::clamp(entry.ratio, 0.1f, 0.9f)
                : 0.5f;
        }

        for (const auto& entry : value.tab_groups) {
            auto* anchor = find_window(entry.anchor);
            if (!anchor || anchor->is_floating())
                continue;
            for (const auto& title : entry.windows) {
                auto* added = find_window(title);
                if (!added || added == anchor || added->is_floating())
                    continue;
                merge_window_as_tab(*anchor, *added);
            }
            const auto group = std::find_if(m_tab_groups.begin(), m_tab_groups.end(), [anchor](const tab_group& candidate) {
                return candidate.anchor == anchor;
            });
            if (group != m_tab_groups.end()) {
                auto* active = find_window(entry.active);
                if (active && std::find(group->windows.begin(), group->windows.end(), active) != group->windows.end())
                    group->active = active;
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
            std::vector<std::pair<window*, float>> roots;
            for (std::size_t window_index = 0; window_index < panel.windows.size(); ++window_index) {
                auto* value = panel.windows[window_index];
                const bool split_child = std::any_of(m_nested_splits.begin(), m_nested_splits.end(), [value](const nested_split& split) {
                    return split.added == value;
                });
                const bool tab_child = std::any_of(m_tab_groups.begin(), m_tab_groups.end(), [value](const tab_group& group) {
                    return group.anchor != value &&
                        std::find(group.windows.begin(), group.windows.end(), value) != group.windows.end();
                });
                if (!split_child && !tab_child) {
                    const float weight = window_index < panel.weights.size() ? panel.weights[window_index] : 1.f;
                    roots.emplace_back(value, weight);
                }
            }
            const auto root_count = roots.size();
            sync_panel_splitters(area, panel, root_count > 0 ? root_count - 1 : 0);

            const bool horizontal = is_horizontal_panel(area);
            panel.split_extent = horizontal ? panel.bounds.z : panel.bounds.w;
            const int total_extent = panel.split_extent;
            const int content_extent = std::max(0, total_extent - m_splitter_size * static_cast<int>(root_count > 0 ? root_count - 1 : 0));
            float weight_sum = std::accumulate(roots.begin(), roots.end(), 0.f, [](const float total, const auto& root) {
                return total + root.second;
            });
            int remaining_extent = content_extent;
            int cursor = horizontal ? panel.bounds.x : panel.bounds.y;
            std::function<void(window*, const vec4i&)> arrange_window;
            arrange_window = [this, &arrange_window, scale](window* value, const vec4i& bounds) {
                auto remaining = bounds;
                for (auto& split : m_nested_splits) {
                    if (split.anchor != value)
                        continue;

                    vec4i added_bounds = remaining;
                    if (split.horizontal) {
                        const int available = std::max(0, remaining.z - m_splitter_size);
                        const int min_extent = std::min(80 * scale, available / 2);
                        const int max_extent = std::max(min_extent, available - min_extent);
                        const int added_width = std::clamp(
                            static_cast<int>(std::round(available * split.ratio)),
                            min_extent, max_extent
                        );
                        const int anchor_width = available - added_width;
                        split.extent = available;
                        if (split.after) {
                            remaining.z = anchor_width;
                            added_bounds.x = remaining.x + anchor_width + m_splitter_size;
                            added_bounds.z = added_width;
                            split.splitter->set_bounds({
                                remaining.x + anchor_width, remaining.y, m_splitter_size, remaining.w
                            });
                        } else {
                            added_bounds.z = added_width;
                            split.splitter->set_bounds({
                                remaining.x + added_width, remaining.y, m_splitter_size, remaining.w
                            });
                            remaining.x += added_width + m_splitter_size;
                            remaining.z = anchor_width;
                        }
                    } else {
                        const int available = std::max(0, remaining.w - m_splitter_size);
                        const int min_extent = std::min(80 * scale, available / 2);
                        const int max_extent = std::max(min_extent, available - min_extent);
                        const int added_height = std::clamp(
                            static_cast<int>(std::round(available * split.ratio)),
                            min_extent, max_extent
                        );
                        const int anchor_height = available - added_height;
                        split.extent = available;
                        if (split.after) {
                            remaining.w = anchor_height;
                            added_bounds.y = remaining.y + anchor_height + m_splitter_size;
                            added_bounds.w = added_height;
                            split.splitter->set_bounds({
                                remaining.x, remaining.y + anchor_height, remaining.z, m_splitter_size
                            });
                        } else {
                            added_bounds.w = added_height;
                            split.splitter->set_bounds({
                                remaining.x, remaining.y + added_height, remaining.z, m_splitter_size
                            });
                            remaining.y += added_height + m_splitter_size;
                            remaining.w = anchor_height;
                        }
                    }
                    split.splitter->set_enable(true);
                    split.splitter->on_update();
                    arrange_window(split.added, added_bounds);
                }

                auto group = std::find_if(m_tab_groups.begin(), m_tab_groups.end(), [value](const tab_group& candidate) {
                    return candidate.anchor == value;
                });
                if (group == m_tab_groups.end()) {
                    value->set_enable(true);
                    value->set_final_rect(remaining.x, remaining.y, remaining.z, remaining.w);
                    value->on_update();
                    return;
                }

                const int tab_height = std::min(28 * scale, std::max(0, remaining.w / 4));
                int tab_x = remaining.x;
                const int tab_right = remaining.x + remaining.z;
                for (std::size_t tab_index = 0; tab_index < group->windows.size(); ++tab_index) {
                    auto* tab_window = group->windows[tab_index];
                    auto* tab = group->buttons[tab_index];
                    const int padding = tab->computed_style.layout.padding.x +
                        tab->computed_style.layout.padding.z;
                    const int margin = tab->computed_style.layout.margin.x +
                        tab->computed_style.layout.margin.z;
                    const int desired_width = std::max(
                        tab->computed_style.layout.limit_min.x,
                        static_cast<int>(std::ceil(
                            tab->get_label().get_text_width(
                                tab->get_label().get_buffer()) + padding + margin
                        ))
                    );
                    const int width = std::min(desired_width, std::max(0, tab_right - tab_x));
                    tab->set_enable(true);
                    tab->set_final_rect(
                        tab_x, remaining.y, width, tab_height
                    );
                    tab->on_update();

                    const auto tab_drag = tab->is_drag();
                    if (tab_drag.x != 0 || tab_drag.y != 0) {
                        group->active = tab_window;
                        focus_window(tab_window);
                        if (m_tab_dragged_window != tab_window) {
                            const auto pointer = bgui::get_mouse_position();
                            m_tab_drag_offset = {
                                pointer.x - tab->processed_x(),
                                pointer.y - tab->processed_y()
                            };
                            m_tab_dragged_window = tab_window;
                        }
                    }

                    const bool active = tab_window == group->active;
                    tab->style.visual.background.normal = active
                        ? bgui::color{0.22f, 0.22f, 0.22f, 1.f}
                        : bgui::color{0.10f, 0.10f, 0.10f, 1.f};
                    tab->style.visual.background.hover = active
                        ? bgui::color{0.28f, 0.28f, 0.28f, 1.f}
                        : bgui::color{0.16f, 0.16f, 0.16f, 1.f};
                    tab->style.visual.background.pressed = active
                        ? bgui::color{0.19f, 0.19f, 0.19f, 1.f}
                        : bgui::color{0.13f, 0.13f, 0.13f, 1.f};
                    tab->mark_style_dirty();
                    tab_window->set_enable(active);
                    if (active) {
                        tab_window->get_title().set_drag(tab_drag);
                        tab_window->set_final_rect(
                            remaining.x, remaining.y + tab_height, remaining.z,
                            std::max(0, remaining.w - tab_height)
                        );
                        tab_window->on_update();
                    }
                    tab->set_drag({0, 0});
                    tab_x += width;
                }
            };

            for (std::size_t window_index = 0; window_index < root_count; ++window_index) {
                const int remaining_count = static_cast<int>(root_count - window_index);
                const int desired = remaining_count == 1 || weight_sum <= 0.f
                    ? remaining_extent
                    : static_cast<int>(std::round(remaining_extent * roots[window_index].second / weight_sum));
                const int min_size = std::min(80 * scale, remaining_extent / remaining_count);
                const int max_size = std::max(min_size, remaining_extent - 80 * scale * (remaining_count - 1));
                const int extent = std::clamp(desired, min_size, max_size);
                const vec4i root_bounds = horizontal
                    ? vec4i{cursor, panel.bounds.y, extent, panel.bounds.w}
                    : vec4i{panel.bounds.x, cursor, panel.bounds.z, extent};
                arrange_window(roots[window_index].first, root_bounds);

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
                weight_sum -= roots[window_index].second;
            }
        }

        for (auto& [lay, elements] : get_elements()) {
            for (auto& element : elements) {
                if (dynamic_cast<dock_splitter*>(element.get()))
                    continue;
                if (element->has_class("dock-drop-zone") ||
                    element->has_class("dock-drop-preview"))
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

        update_drop_targets();

        auto& base_elements = get_elements()[layer::base];
        std::stable_sort(base_elements.begin(), base_elements.end(), [this](const auto& left, const auto& right) {
            const auto z_order = [this](const auto& element) {
                if (element->has_class("dock-splitter"))
                    return 0;
                if (element->has_class("dock-drop-preview"))
                    return 6;
                if (element->has_class("dock-tab"))
                    return 6;
                if (element->has_class("dock-drop-zone"))
                    return 7;

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