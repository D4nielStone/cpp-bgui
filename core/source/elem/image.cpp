#include "elem/image.hpp"

namespace bgui {
    image::image() {
        type = "image";
        m_material.m_use_tex = true;
        m_material.m_shader_tag = "ui::image";
        recives_input(false);
    }

    image::image(const texture& texture)
        : image() {
        set_texture(texture);
    }

    void image::set_texture(const texture& texture) {
        m_texture = texture;
        m_material.m_texture = m_texture;
        mark_style_dirty();
    }

    void image::set_external_texture(
        const unsigned int texture_id,
        const vec2& size,
        const bool flip_vertical
    ) {
        texture external{};
        external.m_path = "external:" + std::to_string(texture_id);
        external.m_id = texture_id;
        external.m_external = true;
        external.m_generate_mipmap = false;
        external.m_size = size;
        set_texture(external);
        m_flip_vertical = flip_vertical;
    }

    void image::get_requires(draw_data* calls) {
        if (m_texture.m_id == 0 && m_texture.m_buffer.empty())
            return;

        set_properties();
        m_material.m_texture = m_texture;
        calls->m_quad_requires.push({
            m_material,
            6,
            vec4{
                static_cast<float>(processed_x()),
                static_cast<float>(processed_y()),
                static_cast<float>(processed_width()),
                static_cast<float>(processed_height())
            },
            m_flip_vertical ? vec2{0.f, 1.f} : vec2{0.f, 0.f},
            m_flip_vertical ? vec2{1.f, 0.f} : vec2{1.f, 1.f}
        });
    }
}
