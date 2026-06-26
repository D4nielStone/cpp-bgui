#include "lay/layout.hpp"
#include "bgui.hpp"

using namespace bgui;

layout::layout() : element() {
    type = "layout";
    recives_input(false);
};

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

void layout::get_requires(bgui::draw_data* data) {
    // the background quad
    element::get_requires(data);
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
};