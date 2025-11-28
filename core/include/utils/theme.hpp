#pragma once
#include "material.hpp"
#include "vec.hpp"

namespace butil {
    enum class mode {
        relative,
        pixel
    };
    enum class orientation {
        vertical,
        horizontal
    };
    enum class alignment {
        start,
        center,
        end,
        stretch
    };
    struct theme{
        butil::color m_clear_color;
        butil::color m_text_color;
        butil::color m_box_color;
        butil::color m_button_color;
        butil::color m_button_border_color;
        butil::color m_button_clicked_color;
        butil::color m_button_hovered_color;
    };
    
    static const theme light_theme = {
        {0.94,0.94,0.94,1.0},
        {0.0,0.0,0.0,1.0},
        {0.92,0.92,0.92,1.0}, 
        {0.9,0.9,0.9,1.0}, 
        {0.6,0.6,0.6,1.0}, 
        {0.88,0.88,0.88,1.0}, 
        {0.88,0.88,0.88,1.0}
    };
    static const theme dark_theme = {
        {0.06f, 0.06f, 0.06f, 1.f},
        {1.f, 1.f, 1.f, 1.f},
        {0.08f, 0.08f, 0.08f, 1.f},
        {0.1f, 0.1f, 0.1f, 1.f},   // released
        {0.04f, 0.04f, 0.04f, 1.f},   // border
        {0.12f, 0.12f, 0.12f, 1.f},   // pressed
        {0.12f, 0.12f, 0.12f, 1.f} // hovered
    };
}; // namespace butil