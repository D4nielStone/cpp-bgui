#pragma once
#include "material.hpp"
#include "vec.hpp"

namespace butil {
    // \brief A draw call structure containing the necessary information to render an element.
    struct draw_request {
        butil::material& m_material;
        GLuint m_vao{0};
        GLenum m_mode{GL_TRIANGLES};
        int m_count{6};
        butil::vec4 m_bounds{0.0f, 0.0f, 100.0f, 100.0f};
        butil::vec2 m_uv_min{0, 0};
        butil::vec2 m_uv_max{0, 0};

        bool operator==(const draw_request& other) const {
            return m_material == other.m_material &&
                   m_vao == other.m_vao &&
                   m_mode == other.m_mode &&
                   m_count == other.m_count;
        }
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
        {0.89f, 0.89f, 0.89f, 1.f},
        {0.f, 0.f, 0.f, 1.f},
        {0.1f, 0.1f, 0.1f, 1.f},
        {0.86, 0.86, 0.86f, 1.f},
        {0.8f, 0.8f, 0.8f, 1.f},
        {0.8f, 0.8f, 0.8f, 1.f},
        {0.8f, 0.8f, 0.8f, 1.f}
    };
    static const theme dark_theme = {
        {0.06f, 0.06f, 0.06f, 1.f},
        {1.f, 1.f, 1.f, 1.f},
        {0.08f, 0.08f, 0.08f, 1.f},
        {0.1f, 0.1f, 0.1f, 1.f},   // released
        {0.04f, 0.04f, 0.04f, 1.f},   // border
        {0.1f, 0.1f, 0.1f, 1.f},   // pressed
        {0.12f, 0.12f, 0.12f, 1.f} // hovered
    };
}; // namespace butil