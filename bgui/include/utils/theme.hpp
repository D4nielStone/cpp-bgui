#pragma once
#include "vec.hpp"

namespace butil {
    struct theme{
        butil::color m_clear_color;
        butil::color m_text_color;
    };
    
    static theme light_theme = {
        {0.86f, 0.86f, 0.86f, 1.f},
        {0.f, 0.f, 0.f, 1.f}
    };
    static theme dark_theme = {
        {0.11f, 0.11f, 0.11f, 1.f},
        {1.f, 1.f, 1.f, 1.f}
    };
}; // namespace butil