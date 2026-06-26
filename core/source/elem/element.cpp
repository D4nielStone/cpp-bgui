#include "elem/element.hpp"
#include <cstdlib>
#include <iostream>
#include "bgui.hpp"
#include "os/style_manager.hpp"

using namespace bgui;
void element::mark_style_dirty() {
    m_style_dirty = true;

    if (auto* lay = as_layout()) {
        lay->mark_children_style_dirty();
    }
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
    m_enabled = b;
}

void element::on_update() {
    set_style_state(state::normal);
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
        bgui::merge(computed_style.visual, get_parent()->style.visual, m_state);
    bgui::merge(computed_style, style, m_state);

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
    set_properties();
    if (!computed_style.visual.visible) return;
    calls->m_quad_requires.push({
        m_material,
        6,
        vec4{
            static_cast<float>(m_rect[0]),
            static_cast<float>(m_rect[1]),
            static_cast<float>(m_rect.z),
            static_cast<float>(m_rect.w)
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

    // Step 6: Set final rect dimensions
    m_rect = { m_rect.x, m_rect.y, static_cast<int>(w), static_cast<int>(h) };
}