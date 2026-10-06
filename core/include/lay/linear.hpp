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
        element m_scrollbar;

        void update_scrollbar_rect();
    public:
        linear(const bgui::orientation& ori = bgui::orientation::horizontal);
        ~linear() = default;
    
        void on_update() override;
        void get_requires(bgui::draw_data* data) override;
        void calc_content_size(const layer& lay) override;
        void set_scrollable(bool enabled);
        int get_scroll_offset() const { return m_scroll_offset; }
        element& get_scrollbar_element() { return m_scrollbar; }
        bool clips_children() const override { return m_scrollable; }
        
        bgui::layout* as_layout() override { return this; }
    };
}// namespace bgui