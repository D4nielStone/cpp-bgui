#pragma once
#include "material.hpp"
#include "vec.hpp"
#include <vector>

namespace butil{
    // \brief A draw call structure containing the necessary information to render an element.
    struct draw_request {
        butil::material& m_material;
        int m_count{6};
        butil::vec4 m_bounds{0.0f, 0.0f, 100.0f, 100.0f};
        butil::vec2 m_uv_min{0, 0};
        butil::vec2 m_uv_max{0, 0};

        bool operator==(const draw_request& other) const {
            return m_material == other.m_material &&
                   m_count == other.m_count && m_uv_max == other.m_uv_max &&
                   m_uv_min == other.m_uv_min;
        }
    };

    struct draw_data {
        std::vector<draw_request> m_quad_requests;
    };
}