#include "lay/layout.hpp"
#include "bgui.hpp"

using namespace bgui;

layout::layout() : element() {
    m_visible = false;
    apply_theme(bgui::get_theme());
};

void layout::update() {
    for(auto& elem : m_elements) elem->update();
}

void layout::fit_to_content() {
    // fit based on absolute layout
    unsigned int max_width = 0, max_height = 0;
    for(auto& elem : m_elements) {
        max_width = std::max(max_width, elem->get_x() + elem->get_width() + m_padding[0]*2);
        max_height = std::max(max_height, elem->get_y() + elem->get_height() + m_padding[1]*2);
    }
    set_size(max_width, max_height);
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
        // adjust the bounds to be modular to this layout
        if(data->m_quad_requests.empty()) continue;
    }
};