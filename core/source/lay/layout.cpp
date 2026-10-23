#include "lay/layout.hpp"
#include "bgui.hpp"
#include "utils/resize_module.hpp"

using namespace bgui;

layout::layout() : element() {
    type = "layout";
    receives_input(false);
};

layout::~layout() {
    bgui::cancel_interactions(this);
}

void layout::on_update() {
    for(auto& [lay, elems] : m_elements) {
        for(auto& elem : elems) {
            if (!elem || !elem->is_enabled())
                continue;
            elem->on_update();
        }
    }
}

void layout::calculate_content_sizes() {
    for (auto& [lay, elems] : m_elements) {
        for (auto& elem : elems) {
            if (!elem || !elem->is_enabled())
                continue;

            if (auto* child_layout = elem->as_layout())
                child_layout->calculate_content_sizes();
            if (elem->is_flex())
                elem->calc_content_size(lay);
        }
        calc_content_size(lay);
    }
}

void layout::arrange() {
    if (!is_layout_dirty())
        return;
    clear_layout_dirty();

    for (auto& [lay, elems] : m_elements) {
        for (auto& elem : elems) {
            if (!elem || !elem->is_enabled())
                continue;
            elem->process_required_size(processed_size());
            elem->arrange();
        }
    }
}

void layout::prepare_geometry_frame() {
    if (!is_layout_dirty())
        return;
    element::prepare_geometry_frame();
    for (auto& [lay, elems] : m_elements) {
        for (auto& elem : elems) {
            if (elem)
                elem->prepare_geometry_frame();
        }
    }
}

void layout::commit_geometry() {
    element::commit_geometry();
    for (auto& [lay, elems] : m_elements) {
        for (auto& elem : elems) {
            if (elem)
                elem->commit_geometry();
        }
    }
}

std::map<layer, std::vector<std::unique_ptr<element>>> &layout::get_elements() {
        return m_elements;
}

void layout::set_resizable(bool enabled) {
    if (m_resizable == enabled) return;
    m_resizable = enabled;
    mark_layout_dirty();
    resize_module::configure(*this, enabled);
}

void layout::get_requires(bgui::draw_data* data) {
    // the background quad
    element::get_requires(data);
    const auto inherited_clip = data->m_clip_rect;
    const bool has_active_clip = inherited_clip.z > 0 && inherited_clip.w > 0;
    if (clips_children())
        data->m_clip_rect = bgui::intersect_rect(inherited_clip, get_children_clip_rect());
    // linear layouts get the draw call in addition order
    for (auto& [lay, elems] : m_elements) {
        for(auto& elem : elems) {
            if(!elem->is_enabled()) continue;
            const auto& clip = data->m_clip_rect;
            const auto rect = elem->processed_rect();
            if (has_active_clip &&
                (clip.z <= 0 || clip.w <= 0 ||
                 rect.x + rect.z <= clip.x || rect.x >= clip.x + clip.z ||
                 rect.y + rect.w <= clip.y || rect.y >= clip.y + clip.w))
                continue;
            // get the requires from each element
            elem->get_cached_requires(data);
            // adjust the rect to be modular to this layout
            if(data->m_quad_requires.empty()) continue;
        }
    }
    data->m_clip_rect = inherited_clip;
};

bool layout::supports_render_cache() const {
    if (!m_cacheability_dirty)
        return m_subtree_cacheable;

    m_subtree_cacheable = true;
    for (const auto& [lay, elems] : m_elements) {
        for (const auto& elem : elems) {
            if (elem && elem->is_enabled() && !elem->supports_render_cache()) {
                m_subtree_cacheable = false;
                break;
            }
        }
        if (!m_subtree_cacheable)
            break;
    }
    m_cacheability_dirty = false;
    return m_subtree_cacheable;
}