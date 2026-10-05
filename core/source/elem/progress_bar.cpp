#include "elem/progress_bar.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
    void validate_range(float minimum, float maximum) {
        if (!std::isfinite(minimum) || !std::isfinite(maximum) || maximum <= minimum)
            throw std::invalid_argument("[BGUI] Progress bar range must be finite and maximum must exceed minimum.");
    }

    void set_rounded_color(
        bgui::material& material,
        const bgui::color& color,
        float radius,
        float border_size
    ) {
        material.set("bg_color", color);
        material.set("border_color", color);
        material.set("bordered", radius > 0.f);
        material.set("border_radius", radius);
        material.set("border_size", border_size);
    }
}

bgui::progress_bar::progress_bar(
    float minimum,
    float maximum,
    float value,
    bgui::orientation direction
) : m_minimum(minimum),
    m_maximum(maximum),
    m_value(minimum),
    m_orientation(direction) {
    validate_range(minimum, maximum);
    if (!std::isfinite(value))
        throw std::invalid_argument("[BGUI] Progress bar value must be finite.");
    if (direction != bgui::orientation::horizontal && direction != bgui::orientation::vertical)
        throw std::invalid_argument("[BGUI] Progress bar orientation is invalid.");
    m_value = std::clamp(value, minimum, maximum);
    type = "progressbar";
    recives_input(false);
}

void bgui::progress_bar::set_range(float minimum, float maximum) {
    validate_range(minimum, maximum);
    m_minimum = minimum;
    m_maximum = maximum;
    m_value = std::clamp(m_value, minimum, maximum);
}

void bgui::progress_bar::set_value(float value) {
    if (!std::isfinite(value))
        throw std::invalid_argument("[BGUI] Progress bar value must be finite.");
    m_value = std::clamp(value, m_minimum, m_maximum);
}

void bgui::progress_bar::get_requires(bgui::draw_data* calls) {
    if (is_style_dirty())
        compute_style();
    if (!computed_style.visual.visible)
        return;

    const float width = static_cast<float>(processed_width());
    const float height = static_cast<float>(processed_height());
    if (width <= 0.f || height <= 0.f)
        return;

    const bool vertical = m_orientation == orientation::vertical;
    const float ratio = static_cast<float>(
        (static_cast<double>(m_value) - m_minimum) /
        (static_cast<double>(m_maximum) - m_minimum)
    );
    const float cross_extent = vertical ? width : height;
    const float thickness = std::min(cross_extent, std::max(2.f, cross_extent * 0.7f));
    const float radius = thickness * 0.5f;
    const float extent = vertical ? height : width;
    const float origin_x = processed_x() + width * 0.5f;
    const float origin_y = processed_y() + height * 0.5f;
    set_rounded_color(
        m_material,
        computed_style.visual.background,
        radius,
        computed_style.visual.border_size
    );
    set_rounded_color(m_fill_material, computed_style.visual.border, radius, 0.f);

    if (vertical) {
        calls->enqueue({
            m_material, 6,
            {origin_x - thickness * 0.5f, static_cast<float>(processed_y()), thickness, height}
        });
        const float fill_height = extent * ratio;
        if (fill_height > 0.f) {
            calls->enqueue({
                m_fill_material, 6,
                {origin_x - thickness * 0.5f,
                 static_cast<float>(processed_y()) + height - fill_height,
                 thickness, fill_height}
            });
        }
    } else {
        calls->enqueue({
            m_material, 6,
            {static_cast<float>(processed_x()), origin_y - thickness * 0.5f, width, thickness}
        });
        const float fill_width = extent * ratio;
        if (fill_width > 0.f) {
            calls->enqueue({
                m_fill_material, 6,
                {static_cast<float>(processed_x()), origin_y - thickness * 0.5f,
                 fill_width, thickness}
            });
        }
    }
}
