#pragma once
#include "vec.hpp"

namespace butil {
    struct theme{
        butil::color m_clear_color;
        butil::color m_text_color;
        butil::color m_box_color;
        butil::color m_button_color;
        butil::color m_button_border_color;
    };
    
    static const theme light_theme = {
        {0.89f, 0.89f, 0.89f, 1.f},
        {0.f, 0.f, 0.f, 1.f},
        {0.1f, 0.1f, 0.1f, 1.f},
        {0.86, 0.86, 0.86f, 1.f},
        {0.8f, 0.8f, 0.8f, 1.f}
    };
    static const theme dark_theme = {
        {0.11f, 0.11f, 0.11f, 1.f},
        {1.f, 1.f, 1.f, 1.f},
        {0.07f, 0.07f, 0.07f, 1.f},
        {0.14f, 0.14f, 0.14f, 1.f},
        {0.2f, 0.2f, 0.2f, 1.f}
    };
}; // namespace butil