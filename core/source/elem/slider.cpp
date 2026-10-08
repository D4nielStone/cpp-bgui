#include "elem/slider.hpp"
#include "bgui.hpp"
#include "os/os.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
    void validate_range(float minimum, float maximum) {
        if (!std::isfinite(minimum) || !std::isfinite(maximum) || maximum <= minimum)
            throw std::invalid_argument("[BGUI] Slider range must be finite and maximum must exceed minimum.");
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

bgui::slider::slider(
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
        throw std::invalid_argument("[BGUI] Slider value must be finite.");
    if (direction != bgui::orientation::horizontal && direction != bgui::orientation::vertical)
        throw std::invalid_argument("[BGUI] Slider orientation is invalid.");
    m_value = normalize_value(value);
    type = "slider";
    receives_input(true);
}

float bgui::slider::normalize_value(float value) const {
    const double clamped = std::clamp(
        static_cast<double>(value),
        static_cast<double>(m_minimum),
        static_cast<double>(m_maximum)
    );
    if (m_step <= 0.f)
        return static_cast<float>(clamped);

    const double steps = std::round(
        (clamped - static_cast<double>(m_minimum)) / static_cast<double>(m_step)
    );
    const double snapped = static_cast<double>(m_minimum) + steps * static_cast<double>(m_step);
    return static_cast<float>(std::clamp(
        snapped,
        static_cast<double>(m_minimum),
        static_cast<double>(m_maximum)
    ));
}

void bgui::slider::set_range(float minimum, float maximum) {
    validate_range(minimum, maximum);
    const float previous_value = m_value;
    m_minimum = minimum;
    m_maximum = maximum;
    m_value = normalize_value(previous_value);
    if (m_value != previous_value && m_on_change)
        m_on_change(m_value);
}

void bgui::slider::set_value(float value) {
    if (!std::isfinite(value))
        throw std::invalid_argument("[BGUI] Slider value must be finite.");

    const float normalized = normalize_value(value);
    if (normalized == m_value)
        return;

    m_value = normalized;
    if (m_on_change)
        m_on_change(m_value);
}

void bgui::slider::set_step(float step) {
    if (!std::isfinite(step) || step < 0.f)
        throw std::invalid_argument("[BGUI] Slider step must be finite and non-negative.");

    const float previous_value = m_value;
    m_step = step;
    m_value = normalize_value(m_value);
    if (m_value != previous_value && m_on_change)
        m_on_change(m_value);
}

void bgui::slider::set_on_change(const std::function<void(float)>& callback) {
    m_on_change = callback;
}

void bgui::slider::update_from_pointer() {
    const int extent = m_orientation == orientation::horizontal
        ? processed_width()
        : processed_height();
    if (extent <= 0)
        return;

    const auto mouse = bgui::get_mouse_position();
    const int origin = m_orientation == orientation::horizontal
        ? processed_x()
        : processed_y();
    const int coordinate = m_orientation == orientation::horizontal ? mouse.x : mouse.y;
    double ratio = std::clamp(
        static_cast<double>(coordinate - origin) / static_cast<double>(extent),
        0.0,
        1.0
    );
    if (m_orientation == orientation::vertical)
        ratio = 1.0 - ratio;

    const double value = static_cast<double>(m_minimum) +
        (static_cast<double>(m_maximum) - static_cast<double>(m_minimum)) * ratio;
    set_value(static_cast<float>(value));
}

void bgui::slider::on_pressed() {
    element::on_pressed();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
}

void bgui::slider::on_clicked() {
    element::on_clicked();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
    update_from_pointer();
}

void bgui::slider::on_released() {
    element::on_released();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
}

void bgui::slider::on_mouse_hover() {
    element::on_mouse_hover();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
}

void bgui::slider::set_drag(const bgui::vec2i& mouse_delta) {
    element::set_drag(mouse_delta);
    update_from_pointer();
}

void bgui::slider::get_requires(bgui::draw_data* calls) {
    if (is_style_dirty())
        compute_style();
    if (!computed_style.visual.visible)
        return;

    const float width = static_cast<float>(processed_width());
    const float height = static_cast<float>(processed_height());
    if (width <= 0.f || height <= 0.f)
        return;

    const bool vertical = m_orientation == orientation::vertical;
    const float extent = vertical ? height : width;
    const float cross_extent = vertical ? width : height;
    const float track_thickness = std::max(2.f, cross_extent * 0.22f);
    const float center_x = processed_x() + width * 0.5f;
    const float center_y = processed_y() + height * 0.5f;
    const float thumb_diameter = cross_extent * 0.7f;
    const float ratio = static_cast<float>(
        (static_cast<double>(m_value) - m_minimum) /
        (static_cast<double>(m_maximum) - m_minimum)
    );

    set_rounded_color(
        m_material,
        computed_style.visual.background,
        track_thickness * 0.5f,
        computed_style.visual.border_size
    );
    set_rounded_color(
        m_fill_material,
        computed_style.visual.border,
        track_thickness * 0.5f,
        0.f
    );
    set_rounded_color(
        m_thumb_material,
        computed_style.visual.border,
        thumb_diameter * 0.5f,
        0.f
    );

    if (vertical) {
        const float thumb_y = processed_y() + (1.f - ratio) * extent;
        calls->enqueue({
            m_material, 6,
            {center_x - track_thickness * 0.5f, static_cast<float>(processed_y()),
             track_thickness, height}
        });
        const float fill_height = (processed_y() + height) - thumb_y;
        if (fill_height > 0.f) {
            calls->enqueue({
                m_fill_material, 6,
                {center_x - track_thickness * 0.5f, thumb_y, track_thickness, fill_height}
            });
        }
        calls->enqueue({
            m_thumb_material, 6,
            {center_x - thumb_diameter * 0.5f, thumb_y - thumb_diameter * 0.5f,
             thumb_diameter, thumb_diameter}
        });
    } else {
        const float thumb_x = processed_x() + ratio * extent;
        calls->enqueue({
            m_material, 6,
            {static_cast<float>(processed_x()), center_y - track_thickness * 0.5f,
             width, track_thickness}
        });
        const float fill_width = thumb_x - processed_x();
        if (fill_width > 0.f) {
            calls->enqueue({
                m_fill_material, 6,
                {static_cast<float>(processed_x()), center_y - track_thickness * 0.5f,
                 fill_width, track_thickness}
            });
        }
        calls->enqueue({
            m_thumb_material, 6,
            {thumb_x - thumb_diameter * 0.5f, center_y - thumb_diameter * 0.5f,
             thumb_diameter, thumb_diameter}
        });
    }
}
