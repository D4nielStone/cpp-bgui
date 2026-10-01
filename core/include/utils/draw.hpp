#pragma once
#include "material.hpp"
#include "vec.hpp"
#include <queue>

namespace bgui{
    // \brief A draw call structure containing the necessary information to render an element.
    struct draw_require {
        bgui::material& m_material;
        int m_count{6};
        bgui::vec4 m_rect{0.f, 0.f, 100.f, 100.f};
        bgui::vec2 m_uv_min{0, 0};
        bgui::vec2 m_uv_max{1, 1};
        bgui::vec4i m_clip_rect{0, 0, 0, 0};

        bool operator==(const draw_require& other) const {
            return m_material == other.m_material &&
                   m_count == other.m_count && m_uv_max == other.m_uv_max &&
                   m_uv_min == other.m_uv_min;
        }
    };

    inline bgui::vec4i intersect_rect(const bgui::vec4i& a, const bgui::vec4i& b) {
        const int left = std::max(a.x, b.x);
        const int top = std::max(a.y, b.y);
        const int right = std::min(a.x + a.z, b.x + b.z);
        const int bottom = std::min(a.y + a.w, b.y + b.w);
        return {left, top, std::max(0, right - left), std::max(0, bottom - top)};
    }

    struct draw_data {
        std::queue<draw_require> m_quad_requires;
        bgui::vec4i m_clip_rect{0, 0, 0, 0};

        void enqueue(draw_require require) {
            require.m_clip_rect = m_clip_rect;
            m_quad_requires.push(require);
        }
    };
}