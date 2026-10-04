/** Dock layout with resizable regions for pinned windows. */
#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "lay/layout.hpp"

namespace bgui {
    class button;
    class window;
    class dock_splitter;

    enum class dock_area {
        left,
        right,
        top,
        bottom,
        center
    };

    class dock final : public layout {
    public:
        struct window_configuration {
            std::string title;
            dock_area area{dock_area::center};
            float weight{1.f};
            bool floating{false};
            vec4i rect{0};
        };

        struct split_configuration {
            std::string anchor;
            std::string added;
            bool horizontal{false};
            bool after{false};
            float ratio{0.5f};
        };

        struct tab_group_configuration {
            std::string anchor;
            std::string active;
            std::vector<std::string> windows;
        };

        struct configuration {
            float left_ratio{0.24f};
            float right_ratio{0.24f};
            float top_ratio{0.24f};
            float bottom_ratio{0.24f};
            std::vector<window_configuration> windows;
            std::vector<split_configuration> splits;
            std::vector<tab_group_configuration> tab_groups;
        };

        dock();

        window& add_window(const std::string& title, dock_area area = dock_area::center);
        bool remove_window(window* value);
        void focus_window(window* value);
        configuration get_configuration();
        void apply_configuration(const configuration& value);
        void on_update() override;

    private:
        struct panel {
            std::vector<window*> windows;
            std::vector<float> weights;
            std::vector<dock_splitter*> splitters;
            vec4i bounds{0};
            int split_extent{0};
        };

        struct nested_split {
            window* anchor{nullptr};
            window* added{nullptr};
            bool horizontal{false};
            bool after{false};
            float ratio{0.5f};
            int extent{0};
            dock_splitter* splitter{nullptr};
        };

        struct tab_group {
            window* anchor{nullptr};
            window* active{nullptr};
            std::vector<window*> windows;
            std::vector<button*> buttons;
        };

        std::array<panel, 5> m_panels;
        std::array<dock_splitter*, 4> m_area_splitters{};
        std::array<element*, 5> m_drop_targets{};
        element* m_drop_preview{nullptr};
        std::vector<nested_split> m_nested_splits;
        std::vector<tab_group> m_tab_groups;
        window* m_focused_window{nullptr};
        window* m_dragged_window{nullptr};
        window* m_tab_dragged_window{nullptr};
        window* m_drop_target_window{nullptr};
        vec2i m_tab_drag_offset{0, 0};
        float m_left_ratio{0.24f};
        float m_right_ratio{0.24f};
        float m_top_ratio{0.24f};
        float m_bottom_ratio{0.24f};
        int m_inner_width{0};
        int m_inner_height{0};
        int m_splitter_size{6};

        panel& get_panel(dock_area area);
        const panel& get_panel(dock_area area) const;
        void register_window(window& value, dock_area area);
        void split_window(window& anchor, window& added, bool horizontal, bool after);
        void merge_window_as_tab(window& anchor, window& added);
        void sync_windows();
        void update_drop_targets();
        void sync_panel_splitters(dock_area area, panel& value, std::size_t count);
        void resize_area(dock_area area, int delta);
        void resize_panel_split(dock_area area, std::size_t index, int delta);
    };
}