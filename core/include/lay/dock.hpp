/** Dock layout with resizable regions for pinned windows. */
#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "lay/layout.hpp"

namespace bgui {
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

        struct configuration {
            float left_ratio{0.24f};
            float right_ratio{0.24f};
            float top_ratio{0.24f};
            float bottom_ratio{0.24f};
            std::vector<window_configuration> windows;
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

        std::array<panel, 5> m_panels;
        std::array<dock_splitter*, 4> m_area_splitters{};
        std::array<element*, 5> m_drop_targets{};
        window* m_focused_window{nullptr};
        window* m_dragged_window{nullptr};
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
        void sync_windows();
        void update_drop_targets();
        void sync_panel_splitters(dock_area area, panel& value, std::size_t count);
        void resize_area(dock_area area, int delta);
        void resize_panel_split(dock_area area, std::size_t index, int delta);
    };
}