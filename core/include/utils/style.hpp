#pragma once
#include "material.hpp"
#include "vec.hpp"

namespace bgui {
    enum class mode {
        percent,
        pixel,
        match_parent,
        wrap_content,
        stretch,
        same
    };
    enum class orientation {
        vertical,
        horizontal
    };
    enum class alignment {
        start,
        center,
        end
    };
    struct style{
        bgui::color m_clear_color = bgui::color(1.f);
        bgui::color m_text_color = bgui::color(1.f);
        bgui::color m_box_color = bgui::color(1.f);
        bgui::color m_button_color = bgui::color(1.f);
        bgui::color m_button_border_color = bgui::color(1.f);
        bgui::color m_button_clicked_color = bgui::color(1.f);
        bgui::color m_button_hovered_color = bgui::color(1.f);
    };
    
    inline const style light_style = {
        {0.94,0.94,0.94,1.0},
        {0.0,0.0,0.0,1.0},
        {0.92,0.92,0.92,1.0}, 
        {0.9,0.9,0.9,1.0}, 
        {0.6,0.6,0.6,1.0}, 
        {0.88,0.88,0.88,1.0}, 
        {0.88,0.88,0.88,1.0}
    };
    inline const style dark_style = {
        {0.06f, 0.06f, 0.06f, 1.f},
        {1.f, 1.f, 1.f, 1.f},
        {0.08f, 0.08f, 0.08f, 1.f},
        {0.1f, 0.1f, 0.1f, 1.f},   // released
        {0.04f, 0.04f, 0.04f, 1.f},   // border
        {0.12f, 0.12f, 0.12f, 1.f},   // pressed
        {0.12f, 0.12f, 0.12f, 1.f} // hovered
    };
}; // namespace bgui