#include "elem/element.hpp"
#include <cstdlib>
#include <iostream>
#include "bgui.hpp"

using namespace bgui;

element::element() : m_has_tag(false) {
    std::cout << "[DEBUG] New element id:" + std::to_string(m_uid.m_uint) + "\n";
}
element::element(const char* tag) : m_has_tag(true) {
    std::cout << "[DEBUG] New element id:" << m_uid.m_str << "\n";
}

// Margin
void element::set_margin(int left, int top, int right, int bottom) {
    m_margin[0] = left;
    m_margin[1] = top;
    m_margin[2] = right;
    m_margin[3] = bottom;
}

void element::set_margin(int x, int y) {
    set_margin(x, y, x, y);
}

// Padding
void element::set_padding(int left, int top, int right, int bottom) {
    m_padding[0] = left;
    m_padding[1] = top;
    m_padding[2] = right;
    m_padding[3] = bottom;
}

void element::set_padding(int x, int y) {
    set_padding(x, y, x, y);
}

// Borders
void element::set_border_size(int x, int y) {
    m_border.m_size[0] = x;
    m_border.m_size[1] = y;
}
void element::set_border_color(const color& c) {
    m_border.m_color = c;
}
void element::set_border_radius(float radius) {
    m_border.m_radius = radius;
}

// Size constraints
void element::set_min_size(int width, int height) {
    m_min_size[0] = width;
    m_min_size[1] = height;
}

void element::set_max_size(int width, int height) {
    m_max_size[0] = width;
    m_max_size[1] = height;
}

// Requested size
void element::request_size(float width, float height) {
    m_requested_size[0] = width;
    m_requested_size[1] = height;
}

void element::request_height(bgui::mode m, float h) {
    m_requested_mode[1] = m;
    m_requested_size[1] = h;
}
void element::request_width(bgui::mode m, float w) {
    m_requested_mode[0] = m;
    m_requested_size[0] = w;
}

// Material / visibility
void element::set_shader_tag(const std::string &shd) {
    m_material.m_shader_tag = shd;
}

void element::set_material(const bgui::material &mhd) {
    m_material = mhd;
}

void element::set_visible(bool visible) {
    m_visible = visible;
}

void element::update() {
    m_last_drag = {0, 0};
}

vec2i bgui::element::get_drag() const {
    return m_last_drag;
}
void element::get_requests(bgui::draw_data* calls) {

    if (!m_visible) return;
    calls->m_quad_requests.push({
        m_material,
        6,
        vec4{
            static_cast<float>(m_rect[0]),
            static_cast<float>(m_rect[1]),
            static_cast<float>(m_rect[2]),
            static_cast<float>(m_rect[3])
        }
    });
}

void element::update_size(const bgui::vec2i& available) {
    // Step 1: resolve requested size
    auto resolve = [&](int vertical, float max) {
        switch(m_requested_mode[vertical]) {
            case bgui::mode::pixel:   return m_requested_size[vertical];
            case bgui::mode::percent: return max * (std::clamp(m_requested_size[vertical], 0.f, 100.f)/100);
            case bgui::mode::match_parent: return max - get_margin()[vertical+2] - get_margin()[vertical];
            case bgui::mode::wrap_content: return vertical ? content_height() : content_width();
            case bgui::mode::stretch: return max - get_margin()[vertical+2] - get_margin()[vertical];
        }
        return 0.f;
    };

    float w = resolve(0, available[0]);
    float h = resolve(1, available[1]);

    // Step 2: enforce min/max rules
    w = std::clamp((int)w, m_min_size[0], m_max_size[0]);
    h = std::clamp((int)h, m_min_size[1], m_max_size[1]);

    m_rect = { m_rect[0], m_rect[1], static_cast<int>(w), static_cast<int>(h) };
}