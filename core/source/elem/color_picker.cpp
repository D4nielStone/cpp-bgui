#include "elem/color_picker.hpp"
#include "bgui.hpp"
#include "os/os.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace {
    constexpr float pi = 3.14159265358979323846f;
    constexpr float triangle_radius = 0.68f;
    constexpr float hue_ring_inner_radius = 0.75f;
    constexpr float hue_ring_outer_radius = 0.92f;
    constexpr int palette_resolution = 128;
    constexpr int marker_resolution = 32;
    std::atomic<unsigned int> next_color_picker_texture_id{1};

    struct palette_pixel {
        std::array<float, 3> triangle_weights{};
        bgui::color ring_color{0.f, 0.f, 0.f, 0.f};
        bool in_triangle{false};
        bool in_hue_ring{false};
    };

    struct point {
        float x;
        float y;
    };

    constexpr std::array<point, 3> triangle_vertices{{
        {0.f, -triangle_radius},
        {triangle_radius * 0.86602540378f, triangle_radius * 0.5f},
        {-triangle_radius * 0.86602540378f, triangle_radius * 0.5f}
    }};

    float normalize_hue(float hue) {
        hue = std::fmod(hue, 360.f);
        return hue < 0.f ? hue + 360.f : hue;
    }

    bgui::color hue_color(float hue) {
        const float sector = normalize_hue(hue) / 60.f;
        const int index = static_cast<int>(sector);
        const float fraction = sector - static_cast<float>(index);
        const float falling = 1.f - fraction;
        switch (index) {
            case 0: return {1.f, fraction, 0.f, 1.f};
            case 1: return {falling, 1.f, 0.f, 1.f};
            case 2: return {0.f, 1.f, fraction, 1.f};
            case 3: return {0.f, falling, 1.f, 1.f};
            case 4: return {fraction, 0.f, 1.f, 1.f};
            default: return {1.f, 0.f, falling, 1.f};
        }
    }

    std::array<float, 3> barycentric_weights(float x, float y) {
        const auto& a = triangle_vertices[0];
        const auto& b = triangle_vertices[1];
        const auto& c = triangle_vertices[2];
        const float denominator =
            (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
        const float first =
            ((b.y - c.y) * (x - c.x) + (c.x - b.x) * (y - c.y)) / denominator;
        const float second =
            ((c.y - a.y) * (x - c.x) + (a.x - c.x) * (y - c.y)) / denominator;
        return {first, second, 1.f - first - second};
    }

    bool inside_triangle(const std::array<float, 3>& weights) {
        constexpr float edge_tolerance = 0.015f;
        return weights[0] >= -edge_tolerance &&
            weights[1] >= -edge_tolerance &&
            weights[2] >= -edge_tolerance;
    }

    const std::vector<palette_pixel>& get_palette_geometry() {
        static const std::vector<palette_pixel> geometry = [] {
            std::vector<palette_pixel> pixels(
                static_cast<std::size_t>(palette_resolution * palette_resolution)
            );
            for (int row = 0; row < palette_resolution; ++row) {
                const float y = 2.f * (static_cast<float>(row) + 0.5f) /
                    static_cast<float>(palette_resolution) - 1.f;
                for (int column = 0; column < palette_resolution; ++column) {
                    const float x = 2.f * (static_cast<float>(column) + 0.5f) /
                        static_cast<float>(palette_resolution) - 1.f;
                    const float distance_squared = x * x + y * y;
                    auto& pixel = pixels[static_cast<std::size_t>(
                        row * palette_resolution + column
                    )];

                    if (distance_squared >= hue_ring_inner_radius * hue_ring_inner_radius &&
                        distance_squared <= hue_ring_outer_radius * hue_ring_outer_radius) {
                        const float angle = std::atan2(y, x) * 180.f / pi;
                        pixel.ring_color = hue_color(270.f - angle);
                        pixel.in_hue_ring = true;
                    } else {
                        pixel.triangle_weights = barycentric_weights(x, y);
                        pixel.in_triangle = inside_triangle(pixel.triangle_weights);
                    }
                }
            }
            return pixels;
        }();
        return geometry;
    }

    bgui::color triangle_color(
        const std::array<float, 3>& weights,
        const bgui::color& hue
    ) {
        return {
            weights[0] + weights[1] * hue.r,
            weights[0] + weights[1] * hue.g,
            weights[0] + weights[1] * hue.b,
            1.f
        };
    }

    unsigned char to_byte(float value) {
        return static_cast<unsigned char>(
            std::lround(std::clamp(value, 0.f, 1.f) * 255.f)
        );
    }

    void color_to_hsv(const bgui::color& value, float& hue, float& saturation, float& brightness) {
        const float maximum = std::max({value.r, value.g, value.b});
        const float minimum = std::min({value.r, value.g, value.b});
        const float delta = maximum - minimum;
        brightness = maximum;
        saturation = maximum == 0.f ? 0.f : delta / maximum;
        if (delta == 0.f)
            return;

        if (maximum == value.r)
            hue = 60.f * std::fmod((value.g - value.b) / delta, 6.f);
        else if (maximum == value.g)
            hue = 60.f * ((value.b - value.r) / delta + 2.f);
        else
            hue = 60.f * ((value.r - value.g) / delta + 4.f);
        hue = normalize_hue(hue);
    }
}

bgui::color_picker::color_picker(const bgui::color& initial_color) {
    type = "color_picker";
    receives_input(true);
    style.layout.require_size(180.f, 180.f);
    m_material.m_use_tex = true;
    m_material.m_shader_tag = "ui::image";
    m_material.m_texture.m_path = "bgui:color-picker";
    m_material.m_texture.m_id = next_color_picker_texture_id.fetch_add(1);
    m_material.m_texture.m_has_alpha = true;
    m_material.m_texture.m_generate_mipmap = false;
    m_marker.m_path = "bgui:color-picker-marker";
    m_marker.m_size = {static_cast<float>(marker_resolution),
                       static_cast<float>(marker_resolution)};
    m_marker.m_has_alpha = true;
    m_marker.m_generate_mipmap = false;
    m_marker.m_revision = 1;
    m_marker.m_buffer.resize(
        static_cast<std::size_t>(marker_resolution * marker_resolution * 4)
    );
    for (int row = 0; row < marker_resolution; ++row) {
        for (int column = 0; column < marker_resolution; ++column) {
            const float x = (static_cast<float>(column) + 0.5f) /
                marker_resolution * 2.f - 1.f;
            const float y = (static_cast<float>(row) + 0.5f) /
                marker_resolution * 2.f - 1.f;
            const float distance_squared = x * x + y * y;
            bgui::color marker_color{0.f, 0.f, 0.f, 0.f};
            if (distance_squared <= 0.45f * 0.45f) {
                marker_color = distance_squared >= 0.34f * 0.34f
                    ? bgui::color{0.f, 0.f, 0.f, 1.f}
                    : distance_squared >= 0.24f * 0.24f
                        ? bgui::color{1.f, 1.f, 1.f, 1.f}
                        : bgui::color{0.f, 0.f, 0.f, 0.f};
            }
            const std::size_t offset = static_cast<std::size_t>(
                (row * marker_resolution + column) * 4
            );
            m_marker.m_buffer[offset] = to_byte(marker_color.r);
            m_marker.m_buffer[offset + 1] = to_byte(marker_color.g);
            m_marker.m_buffer[offset + 2] = to_byte(marker_color.b);
            m_marker.m_buffer[offset + 3] = to_byte(marker_color.a);
        }
    }
    m_marker_material.m_use_tex = true;
    m_marker_material.m_shader_tag = "ui::image";
    m_marker_material.m_texture = m_marker;
    set_color(initial_color);
}

void bgui::color_picker::set_color(const bgui::color& value) {
    for (std::size_t component = 0; component < 4; ++component) {
        if (!std::isfinite(value[component]))
            throw std::invalid_argument("[BGUI] Color picker color components must be finite.");
    }

    const bgui::color previous_color = m_color;
    const bgui::color normalized{
        std::clamp(value.r, 0.f, 1.f),
        std::clamp(value.g, 0.f, 1.f),
        std::clamp(value.b, 0.f, 1.f),
        std::clamp(value.a, 0.f, 1.f)
    };
    float hue = m_hue;
    float saturation = 0.f;
    float brightness = 0.f;
    color_to_hsv(normalized, hue, saturation, brightness);

    const float previous_hue = m_hue;
    if (saturation > 0.f)
        m_hue = hue;
    m_white_weight = (1.f - saturation) * brightness;
    m_hue_weight = saturation * brightness;
    m_black_weight = 1.f - brightness;
    m_color.a = normalized.a;
    if (m_hue != previous_hue)
        m_palette_dirty = true;
    update_color();
    if (m_color != previous_color && m_on_change)
        m_on_change(m_color);
}

void bgui::color_picker::set_hue(float hue) {
    if (!std::isfinite(hue))
        throw std::invalid_argument("[BGUI] Color picker hue must be finite.");

    const float normalized = normalize_hue(hue);
    if (normalized == m_hue)
        return;

    const bgui::color previous_color = m_color;
    m_hue = normalized;
    m_palette_dirty = true;
    update_color();
    if (m_color != previous_color && m_on_change)
        m_on_change(m_color);
}

void bgui::color_picker::set_on_change(
    const std::function<void(const bgui::color&)>& callback
) {
    m_on_change = callback;
}

void bgui::color_picker::update_color() {
    const bgui::color pure_hue = hue_color(m_hue);
    m_color.r = m_white_weight + m_hue_weight * pure_hue.r;
    m_color.g = m_white_weight + m_hue_weight * pure_hue.g;
    m_color.b = m_white_weight + m_hue_weight * pure_hue.b;
}

void bgui::color_picker::update_from_pointer() {
    const float side = static_cast<float>(std::min(processed_width(), processed_height()));
    if (side <= 0.f)
        return;

    const float radius = side * 0.5f;
    const float center_x = processed_x() + processed_width() * 0.5f;
    const float center_y = processed_y() + processed_height() * 0.5f;
    const auto mouse = bgui::get_mouse_position();
    const float x = (static_cast<float>(mouse.x) - center_x) / radius;
    const float y = (static_cast<float>(mouse.y) - center_y) / radius;

    if (m_drag_target == drag_target::hue) {
        const float degrees = 270.f - std::atan2(y, x) * 180.f / pi;
        set_hue(degrees);
    } else if (m_drag_target == drag_target::triangle) {
        auto weights = barycentric_weights(x, y);
        for (float& weight : weights)
            weight = std::max(0.f, weight);
        const float total = weights[0] + weights[1] + weights[2];
        if (total <= 0.f)
            return;
        for (float& weight : weights)
            weight /= total;

        const bgui::color previous_color = m_color;
        m_white_weight = weights[0];
        m_hue_weight = weights[1];
        m_black_weight = weights[2];
        update_color();
        if (m_color != previous_color && m_on_change)
            m_on_change(m_color);
    }
}

void bgui::color_picker::on_pressed() {
    element::on_pressed();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;

    const float side = static_cast<float>(std::min(processed_width(), processed_height()));
    if (side <= 0.f)
        return;

    const float radius = side * 0.5f;
    const float center_x = processed_x() + processed_width() * 0.5f;
    const float center_y = processed_y() + processed_height() * 0.5f;
    const auto mouse = bgui::get_mouse_position();
    const float x = (static_cast<float>(mouse.x) - center_x) / radius;
    const float y = (static_cast<float>(mouse.y) - center_y) / radius;
    const float distance = std::sqrt(x * x + y * y);

    if (distance >= hue_ring_inner_radius && distance <= hue_ring_outer_radius)
        m_drag_target = drag_target::hue;
    else if (inside_triangle(barycentric_weights(x, y)))
        m_drag_target = drag_target::triangle;
    else
        m_drag_target = drag_target::none;

    update_from_pointer();
}

void bgui::color_picker::on_released() {
    element::on_released();
    m_drag_target = drag_target::none;
}

void bgui::color_picker::on_mouse_hover() {
    element::on_mouse_hover();
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
}

void bgui::color_picker::set_drag(const bgui::vec2i& mouse_delta) {
    element::set_drag(mouse_delta);
    bgui::get_context().m_actual_cursor = bgui::cursor::hand;
    update_from_pointer();
}

void bgui::color_picker::update_palette() {
    const bgui::color pure_hue = hue_color(m_hue);
    auto& palette = m_material.m_texture;
    palette.m_size = {static_cast<float>(palette_resolution),
                      static_cast<float>(palette_resolution)};
    palette.m_buffer.resize(
        static_cast<std::size_t>(palette_resolution * palette_resolution * 4)
    );

    const auto& geometry = get_palette_geometry();
    for (std::size_t index = 0; index < geometry.size(); ++index) {
        const auto& pixel_geometry = geometry[index];
        bgui::color pixel{0.f, 0.f, 0.f, 0.f};
        if (pixel_geometry.in_hue_ring) {
            pixel = pixel_geometry.ring_color;
        } else if (pixel_geometry.in_triangle) {
            pixel = triangle_color(pixel_geometry.triangle_weights, pure_hue);
        }

        const std::size_t offset = index * 4;
        palette.m_buffer[offset] = to_byte(pixel.r);
        palette.m_buffer[offset + 1] = to_byte(pixel.g);
        palette.m_buffer[offset + 2] = to_byte(pixel.b);
        palette.m_buffer[offset + 3] = to_byte(pixel.a);
    }
    ++palette.m_revision;
    m_palette_dirty = false;
}

void bgui::color_picker::get_requires(bgui::draw_data* calls) {
    if (is_style_dirty())
        compute_style();
    if (!computed_style.visual.visible)
        return;

    const int side = std::min(processed_width(), processed_height());
    if (side <= 0)
        return;
    if (m_palette_dirty)
        update_palette();

    set_properties();
    calls->enqueue({
        m_material,
        6,
        {
            processed_x() + (processed_width() - side) * 0.5f,
            processed_y() + (processed_height() - side) * 0.5f,
            static_cast<float>(side),
            static_cast<float>(side)
        }
    });

    const float marker_size = side * 0.07f;
    const float origin_x = processed_x() + (processed_width() - side) * 0.5f;
    const float origin_y = processed_y() + (processed_height() - side) * 0.5f;
    const float center_x = origin_x + side * 0.5f;
    const float center_y = origin_y + side * 0.5f;
    const float hue_angle = (270.f - m_hue) * pi / 180.f;
    const float hue_marker_x = center_x + std::cos(hue_angle) * side * 0.4175f;
    const float hue_marker_y = center_y + std::sin(hue_angle) * side * 0.4175f;
    const float selected_x =
        m_white_weight * triangle_vertices[0].x +
        m_hue_weight * triangle_vertices[1].x +
        m_black_weight * triangle_vertices[2].x;
    const float selected_y =
        m_white_weight * triangle_vertices[0].y +
        m_hue_weight * triangle_vertices[1].y +
        m_black_weight * triangle_vertices[2].y;
    const float triangle_marker_x = center_x + selected_x * side * 0.5f;
    const float triangle_marker_y = center_y + selected_y * side * 0.5f;

    calls->enqueue({
        m_marker_material,
        6,
        {hue_marker_x - marker_size * 0.5f,
         hue_marker_y - marker_size * 0.5f,
         marker_size, marker_size}
    });
    calls->enqueue({
        m_marker_material,
        6,
        {triangle_marker_x - marker_size * 0.5f,
         triangle_marker_y - marker_size * 0.5f,
         marker_size, marker_size}
    });
}
