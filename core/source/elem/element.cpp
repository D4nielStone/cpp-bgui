#include "elem/element.hpp"
#include <cstdlib>
#include <iostream>
#include "bgui.hpp"
#include "os/style_manager.hpp"

using namespace bgui;

namespace {
    bool s_geometry_frame_active = false;
    bool s_arranging_geometry = false;
}

element::~element() {
    bgui::cancel_interactions(this);
}

void element::begin_geometry_frame() {
    s_geometry_frame_active = true;
    s_arranging_geometry = false;
}

void element::begin_arranging() {
    s_arranging_geometry = true;
}

void element::end_arranging() {
    s_arranging_geometry = false;
}

void element::end_geometry_frame() {
    s_arranging_geometry = false;
    s_geometry_frame_active = false;
}

int element::processed_x() const {
    return s_arranging_geometry ? m_arranged_rect.x : m_rect.x;
}

int element::processed_y() const {
    return s_arranging_geometry ? m_arranged_rect.y : m_rect.y;
}

int element::processed_width() const {
    return s_arranging_geometry ? m_arranged_rect.z : m_rect.z;
}

int element::processed_height() const {
    return s_arranging_geometry ? m_arranged_rect.w : m_rect.w;
}

void element::set_final_rect(const int x, const int y, const int w, const int h) {
    const vec4i next_rect{x, y, w, h};
    if (m_arranged_rect == next_rect && (s_geometry_frame_active || m_rect == next_rect))
        return;
    m_arranged_rect = {x, y, w, h};
    if (!s_geometry_frame_active)
        m_rect = m_arranged_rect;
    mark_layout_dirty();
}

void element::set_position(const int x, const int y) {
    if (m_arranged_rect.x == x && m_arranged_rect.y == y)
        return;
    m_arranged_rect.x = x;
    m_arranged_rect.y = y;
    if (!s_geometry_frame_active) {
        m_rect.x = x;
        m_rect.y = y;
    }
    mark_layout_dirty();
}

void element::set_final_size(const int width, const int height) {
    if (m_arranged_rect.z == width && m_arranged_rect.w == height)
        return;
    m_arranged_rect.z = width;
    m_arranged_rect.w = height;
    if (!s_geometry_frame_active) {
        m_rect.z = width;
        m_rect.w = height;
    }
    mark_layout_dirty();
}

vec2i element::processed_position() const {
    return {processed_x(), processed_y()};
}

vec2i element::processed_size() const {
    return {processed_width(), processed_height()};
}

vec4i element::processed_rect() const {
    return {
        processed_x(), processed_y(), processed_width(), processed_height()
    };
}

void element::prepare_geometry_frame() {
    m_arranged_rect = m_rect;
}

void element::commit_geometry() {
    m_rect = m_arranged_rect;
}

void element::mark_style_dirty() {
    m_style_dirty = true;
    mark_layout_dirty();

    if (auto* lay = as_layout()) {
        lay->mark_children_style_dirty();
    }
}

void element::mark_layout_dirty() {
    if (s_arranging_geometry)
        return;

    m_layout_dirty = true;
    if (m_parent)
        m_parent->mark_layout_dirty();
}

void element::add_class(const std::string& cls) {
    if (std::find(classes.begin(), classes.end(), cls) != classes.end())
        return;

    classes.push_back(cls);
    mark_style_dirty();
}

void element::remove_class(const std::string& cls) {
    auto it = std::remove(classes.begin(), classes.end(), cls);
    if (it != classes.end()) {
        classes.erase(it, classes.end());
        mark_style_dirty();
    }
}

bool element::has_class(const std::string& cls) const {
    return std::find(classes.begin(), classes.end(), cls) != classes.end();
}

void element::clear_classes() {
    if (!classes.empty()) {
        classes.clear();
        mark_style_dirty();
    }
}

void element::set_enable(bool b){
    if (m_enabled == b)
        return;

    m_enabled = b;
    mark_layout_dirty();
    if (!m_enabled) {
        bgui::cancel_interactions(this);
        set_style_state(state::disabled);
    } else if (get_style_state() == state::disabled) {
        set_style_state(state::normal);
    }
}

void element::on_update() {
}

void element::arrange() {
}

vec2i bgui::element::is_drag() const {
    return m_last_drag;
}

void element::compute_style() {
    auto& sm = style_manager::get_instance();
    
    computed_style = {};
    // compute it's style
    sm.resolve(
        computed_style,
        type,
        classes,
        id,
        m_state
    );

    // inherit parent's style
    if(get_parent())
        bgui::merge(computed_style.visual, get_parent()->style.visual, m_state, false);
    bgui::merge(computed_style, style, m_state);

    const float scale = bgui::get_global_scale();
    for (size_t axis = 0; axis < 2; ++axis) {
        if (computed_style.layout.size_mode[axis] == bgui::mode::pixel) {
            computed_style.layout.size[axis] *= scale;
        }
        computed_style.layout.limit_min[axis] =
            static_cast<int>(computed_style.layout.limit_min[axis] * scale);
        if (computed_style.layout.limit_max[axis] != INT_MAX) {
            computed_style.layout.limit_max[axis] =
                static_cast<int>(computed_style.layout.limit_max[axis] * scale);
        }
    }
    for (size_t edge = 0; edge < 4; ++edge) {
        computed_style.layout.padding[edge] =
            static_cast<int>(computed_style.layout.padding[edge] * scale);
        computed_style.layout.margin[edge] =
            static_cast<int>(computed_style.layout.margin[edge] * scale);
    }
    computed_style.visual.border_radius *= scale;
    computed_style.visual.border_size *= scale;

    clear_style_dirty();
}
void element::set_properties() {
    m_material.set("bg_color", computed_style.visual.background);
    m_material.set("border_color", computed_style.visual.border);
    m_material.set("bordered", computed_style.visual.border_radius > 0.f ? true : false);
    m_material.set("bordered", computed_style.visual.border.a > 0.f ? true : false);
    m_material.set("border_radius", computed_style.visual.border_radius);
    m_material.set("border_size", computed_style.visual.border_size);
    m_material.set("text_color", computed_style.visual.text);
}
void element::get_requires(bgui::draw_data* calls) {
    // Ensure the computed style is up-to-date for this element before emitting draw calls.
    if (is_style_dirty())
        compute_style();

    set_properties();
    if (!computed_style.visual.visible) return;
    calls->enqueue({
        m_material,
        6,
        vec4{
            static_cast<float>(processed_x()),
            static_cast<float>(processed_y()),
            static_cast<float>(processed_width()),
            static_cast<float>(processed_height())
        }
    });
}

void element::process_required_size(const bgui::vec2i& available) {
    auto resolve = [&](int vertical, float max) {
        switch(computed_style.layout.size_mode[vertical]) {
            case bgui::mode::pixel:
                return computed_style.layout.size[vertical];
            case bgui::mode::percent:
                return max * (std::clamp(computed_style.layout.size[vertical], 0.f, 100.f) / 100.f);
            case bgui::mode::match_parent:
                return max;
            case bgui::mode::wrap_content:
                return (float)get_content_size()[vertical];
            case bgui::mode::stretch:
                return max;
        }
        return 0.f;
    };

    float w = resolve(0, available[0]);
    float h = resolve(1, available[1]);

    // Step 4: Handle 'same' mode (square elements)
    if (computed_style.layout.size_mode[0] == bgui::mode::same)
        w = h;
    if (computed_style.layout.size_mode[1] == bgui::mode::same)
        h = w;

    // Step 5: Enforce min/max constraints
    w = std::clamp(static_cast<int>(w), 
                   computed_style.layout.limit_min[0], 
                   computed_style.layout.limit_max[0]);
    h = std::clamp(static_cast<int>(h), 
                   computed_style.layout.limit_min[1], 
                   computed_style.layout.limit_max[1]);

    // Step 6: Set final rect dimensions into arranged rect when in geometry frame,
    // otherwise publish directly into the visible rect.
    const int next_width = static_cast<int>(w);
    const int next_height = static_cast<int>(h);
    const bool size_changed =
        m_arranged_rect.z != next_width || m_arranged_rect.w != next_height;
    m_arranged_rect.z = next_width;
    m_arranged_rect.w = next_height;
    if (!s_geometry_frame_active) {
        m_rect.z = m_arranged_rect.z;
        m_rect.w = m_arranged_rect.w;
    }
    if (size_changed)
        mark_layout_dirty();
}