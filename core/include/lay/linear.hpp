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
        alignment m_cross_alignment {alignment::start},
        m_alignment                 {alignment::start};
    public:
        linear(const bgui::orientation& ori = bgui::orientation::horizontal);
        ~linear() = default;
    
        void on_update() override;
        float content_height() override;
        float content_width() override;
        void set_orientation(const orientation& ori) { m_orientation = ori; };
        void set_alignment(const alignment& align) { m_alignment = align; };
        void set_cross_alignment(const alignment& align) { m_cross_alignment = align; };
        
        bgui::layout* as_layout() override { return this; }
    };
}// namespace bgui