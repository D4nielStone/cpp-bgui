#include "elem/image.hpp"
#include "os/asset_manager.hpp"
#include "bgui.hpp"

namespace bgui {
    image::image() {
        type = "image";
        m_material.m_use_tex = true;
        m_material.m_shader_tag = "ui::image";
        recives_input(false);
        style.layout.require_mode(mode::wrap_content, mode::wrap_content);
    }

    image::image(const texture& texture, std::function<void(const std::string&)> on_click) : image() {
        m_on_click = on_click;
        set_texture(texture);
    }

    image::image(const std::string& texture_path, std::function<void(const std::string&)> on_click) : image() {
        m_on_click = on_click;
        set_texture(asset_manager::get_instance().load_texture(texture_path));
    }

    void image::set_texture(const texture& texture) {
        m_texture = texture;
        m_material.m_texture = m_texture;
        mark_style_dirty();
    }

    void image::set_size(const float width, const float height) {
        style.layout.require_size(width, height);
        style.layout.require_mode(mode::pixel, mode::pixel);
        mark_style_dirty();
    }

    void image::set_size_mode(const mode width_mode, const mode height_mode) {
        style.layout.require_mode(width_mode, height_mode);
        mark_style_dirty();
    }

    void image::set_uv_region(const vec2& uv_min, const vec2& uv_max) {
        m_uv_min = uv_min;
        m_uv_max = uv_max;
    }

    void image::use_natural_size() {
        style.layout.require_mode(mode::wrap_content, mode::wrap_content);
        mark_style_dirty();
    }

    void image::calc_content_size(const layer&) {
        const float scale = bgui::get_global_scale();
        set_content_size(vec2i{
            static_cast<int>(m_texture.m_size[0] * scale),
            static_cast<int>(m_texture.m_size[1] * scale)
        });
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
        // Do not enqueue an image until it has valid pixel data or an external texture ID.
        if (m_texture.m_id == 0 && m_texture.m_buffer.empty())
            return;

        set_properties();
        m_material.m_texture = m_texture;
        calls->enqueue({
            m_material,
            6,
            vec4{
                static_cast<float>(processed_x()),
                static_cast<float>(processed_y()),
                static_cast<float>(processed_width()),
                static_cast<float>(processed_height())
            },
            m_flip_vertical ? vec2{m_uv_min[0], m_uv_max[1]} : m_uv_min,
            m_flip_vertical ? vec2{m_uv_max[0], m_uv_min[1]} : m_uv_max
        });
    }
}
