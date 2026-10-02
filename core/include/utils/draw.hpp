#pragma once
#include "material.hpp"
#include "vec.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <vector>

namespace bgui{
    struct draw_vertex {
        bgui::vec2 m_position;
        bgui::vec4 m_color;
    };

    class draw_list {
        bgui::vec4 m_color{1.f, 1.f, 1.f, 1.f};
        std::vector<draw_vertex> m_vertices;

        void add_triangle_with_color(const bgui::vec2& a, const bgui::vec2& b,
                                     const bgui::vec2& c, const bgui::vec4& color) {
            m_vertices.push_back({a, color});
            m_vertices.push_back({b, color});
            m_vertices.push_back({c, color});
        }

    public:
        void set_color(const bgui::vec4& color) { m_color = color; }
        const bgui::vec4& get_color() const { return m_color; }
        const std::vector<draw_vertex>& get_vertices() const { return m_vertices; }
        void clear() { m_vertices.clear(); }

        void add_line(const bgui::vec2& start, const bgui::vec2& end, float thickness = 1.f) {
            const auto delta = end - start;
            const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            if (length <= 0.f || thickness <= 0.f) return;
            const bgui::vec2 normal{-delta.y / length * thickness * 0.5f,
                                     delta.x / length * thickness * 0.5f};
            add_quad(start + normal, end + normal, end - normal, start - normal);
        }

        void add_polyline(const std::vector<bgui::vec2>& points, float thickness = 1.f,
                          bool closed = false) {
            if (points.size() < 2) return;
            for (size_t i = 1; i < points.size(); ++i)
                add_line(points[i - 1], points[i], thickness);
            if (closed) add_line(points.back(), points.front(), thickness);
        }

        void add_triangle(const bgui::vec2& a, const bgui::vec2& b, const bgui::vec2& c) {
            add_triangle_with_color(a, b, c, m_color);
        }

        void add_quad(const bgui::vec2& a, const bgui::vec2& b,
                      const bgui::vec2& c, const bgui::vec2& d) {
            add_triangle(a, b, c);
            add_triangle(a, c, d);
        }

        void add_circle_filled(const bgui::vec2& center, float radius, int segments = 32) {
            if (radius <= 0.f) return;
            segments = std::max(3, segments);
            constexpr float tau = 6.28318530717958647692f;
            for (int i = 0; i < segments; ++i) {
                const float angle_a = tau * static_cast<float>(i) / segments;
                const float angle_b = tau * static_cast<float>(i + 1) / segments;
                add_triangle(center,
                    {center.x + std::cos(angle_a) * radius, center.y + std::sin(angle_a) * radius},
                    {center.x + std::cos(angle_b) * radius, center.y + std::sin(angle_b) * radius});
            }
        }

        void add_circle(const bgui::vec2& center, float radius, int segments = 32,
                        float thickness = 1.f) {
            if (radius <= 0.f) return;
            segments = std::max(3, segments);
            std::vector<bgui::vec2> points;
            points.reserve(static_cast<size_t>(segments));
            constexpr float tau = 6.28318530717958647692f;
            for (int i = 0; i < segments; ++i) {
                const float angle = tau * static_cast<float>(i) / segments;
                points.push_back({center.x + std::cos(angle) * radius,
                                  center.y + std::sin(angle) * radius});
            }
            add_polyline(points, thickness, true);
        }

        void add_rect_filled(const bgui::vec2& min, const bgui::vec2& max) {
            add_quad(min, {max.x, min.y}, max, {min.x, max.y});
        }

        void add_rect(const bgui::vec2& min, const bgui::vec2& max, float thickness = 1.f) {
            const bgui::vec2 top_right{max.x, min.y};
            const bgui::vec2 bottom_left{min.x, max.y};
            add_polyline({min, top_right, max, bottom_left}, thickness, true);
        }

        void add_convexpolyfilled(const std::vector<bgui::vec2>& points) {
            if (points.size() < 3) return;
            for (size_t i = 1; i + 1 < points.size(); ++i)
                add_triangle(points[0], points[i], points[i + 1]);
        }
    };

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
        bgui::draw_list m_draw_list;
        bgui::vec4i m_clip_rect{0, 0, 0, 0};

        void enqueue(draw_require require) {
            require.m_clip_rect = m_clip_rect;
            m_quad_requires.push(require);
        }
    };
}