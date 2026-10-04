#include "lay/layout.hpp"
#include "bgui.hpp"
#include "utils/resize_module.hpp"

using namespace bgui;

layout::layout() : element() {
    type = "layout";
    recives_input(false);
};

layout::~layout() {
    bgui::cancel_interactions(this);
}

void layout::on_update() {
    for(auto& [lay, elems] : m_elements) {
        for(auto& elem : elems) {
            if(!elem->is_enabled()) continue;
            elem->process_required_size(processed_size());
            elem->on_update();
        }
    }
}

std::map<layer, std::vector<std::unique_ptr<element>>> &layout::get_elements() {
        return m_elements;
}

void layout::set_resizable(bool enabled) {
    if (m_resizable == enabled) return;
    m_resizable = enabled;
    resize_module::configure(*this, enabled);
}

void layout::get_requires(bgui::draw_data* data) {
    // the background quad
    element::get_requires(data);
    const auto inherited_clip = data->m_clip_rect;
    if (clips_children())
        data->m_clip_rect = bgui::intersect_rect(inherited_clip, get_children_clip_rect());
    // linear layouts get the draw call in addition order
    for (auto& [lay, elems] : m_elements) {
        for(auto& elem : elems) {
            if(!elem->is_enabled()) continue;
            // get the requires from each element
            elem->get_requires(data);
            // adjust the rect to be modular to this layout
            if(data->m_quad_requires.empty()) continue;
        }
    }
    data->m_clip_rect = inherited_clip;
};