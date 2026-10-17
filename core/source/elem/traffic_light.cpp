#include "elem/traffic_light.hpp"
#include "bgui.hpp"

void bgui::traffic_light::get_requires(draw_data *calls)
 {
        const auto previous_state = get_style_state();

        if (is_style_dirty())
            compute_style();

        if (!computed_style.visual.visible)
            return;

        const float scale = get_global_scale();
        const float diameter = 12.f * scale;
        const float radius = diameter * 0.5f;

        const vec2 center{
            processed_x() + processed_width() * 0.5f,
            processed_y() + processed_height() * 0.5f
        };

        vec4 color;

        switch (m_type) {
            case traffic_light_type::close:
                color = {1.f, 0.373f, 0.341f, 1.f};
                break;
            case traffic_light_type::minimize:
                color = {0.996f, 0.737f, 0.180f, 1.f};
                break;
            case traffic_light_type::maximize:
                color = {0.157f, 0.784f, 0.251f, 1.f};
                break;
        }

        calls->m_draw_list.set_color(color);
        calls->m_draw_list.add_circle_filled(
            center,
            radius,
            32
        );

        if (previous_state == state::hover ||
            previous_state == state::pressed) {
            calls->m_draw_list.set_color(
                {0.25f, 0.15f, 0.15f, 0.9f}
            );

            const float offset = radius * 0.3f;

            if (m_type == traffic_light_type::close) {
                calls->m_draw_list.add_line(
                    {center.x - offset, center.y - offset},
                    {center.x + offset, center.y + offset},
                    1.f
                );
                calls->m_draw_list.add_line(
                    {center.x + offset, center.y - offset},
                    {center.x - offset, center.y + offset},
                    1.f
                );
            } else if (m_type == traffic_light_type::minimize) {
                calls->m_draw_list.add_line(
                    {center.x - offset, center.y},
                    {center.x + offset, center.y},
                    1.f
                );
            } else {
                calls->m_draw_list.add_line(
                    {center.x - offset, center.y + offset},
                    {center.x + offset, center.y - offset},
                    1.f
                );
            }
        }
    }