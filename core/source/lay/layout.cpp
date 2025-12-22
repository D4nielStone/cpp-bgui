#include "lay/layout.hpp"
#include "bgui.hpp"

using namespace bgui;

layout::layout() : element() {
    m_visible = false;
    recives_input(false);
    apply_style(bgui::get_style());
};

void layout::update() {
    for(auto& elem : m_elements) {
        elem->update_size(processed_size());
        elem->update();
    }
}

std::vector<std::unique_ptr<element>> &layout::get_elements() {
        return m_elements;
    }

void layout::get_requests(bgui::draw_data* data) {
    // the background quad
    element::get_requests(data);
    // linear layouts get the draw call in addition order
    for (auto& elem : m_elements) {
        // get the requests from each element
        elem->get_requests(data);
        // adjust the rect to be modular to this layout
        if(data->m_quad_requests.empty()) continue;
    }
};