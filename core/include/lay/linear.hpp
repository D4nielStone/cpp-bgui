/**
 * @class Linear Layout
 * @file linear.hpp
 */

#pragma once
#include "layout.hpp"

namespace bgui {
    class linear : public layout {
    protected:
        orientation m_orientation;
        bool m_scrollable{false};
        int m_scroll_offset{0};
        int m_scroll_content_height{0};
        float m_scroll_remainder{0.f};
        element m_scrollbar;
        int m_scroll_offset_x{0}, m_scroll_content_width{0};
        float m_scroll_remainder_x{0.f};
        element m_scrollbar_x;

        bool can_scroll_in_direction(float scroll_delta, int axis = 1) const;
        bool has_scrollable_descendant_at(
            float x,
            float y,
            float scroll_delta, int axis = 1
        );
        void update_scrollbar_rect();
    public:
        linear(const bgui::orientation& ori = bgui::orientation::horizontal);
        ~linear() = default;
    
        void on_update() override;
        void arrange() override;
        void prepare_geometry_frame() override;
        void commit_geometry() override;
        void get_requires(bgui::draw_data* data) override;
        void calc_content_size(const layer& lay) override;
        void set_scrollable(bool enabled);
        int get_scroll_offset() const { return m_scroll_offset; }
        element& get_scrollbar_element() { return m_scrollbar; }
        int get_horizontal_scroll_offset() const { return m_scroll_offset_x; }
        element& get_horizontal_scrollbar_element() { return m_scrollbar_x; }
        bool clips_children() const override { return m_scrollable; }
        
        bgui::layout* as_layout() override { return this; }
    };
}// namespace bgui