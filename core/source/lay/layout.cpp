#include "lay/layout.hpp"
#include "bgui.hpp"

using namespace blay;

layout::layout() : element() {
    m_visible = false;
    apply_theme(bgui::instance().get_theme());
};

void layout::update() {
    if(!m_modals.empty()) {
        m_modals.front()->update();
        m_modals.front()->fit_to_content();
        m_modals.front()->set_position(
            get_x() + get_width()/2 - m_modals.front()->get_width()/2,
            get_y() + get_height()/2 - m_modals.front()->get_height()/2
        );
    }
    for(auto& elem : m_elements) elem->update();
}

void layout::fit_to_content() {
    // fit based on absolute layout
    int max_width = 0, max_height = 0;
    for(auto& elem : m_elements) {
        max_width = std::max(max_width, elem->get_x() + elem->get_width());
        max_height = std::max(max_height, elem->get_y() + elem->get_height());
    }
    set_size(max_width, max_height);
}

std::vector<std::unique_ptr<element>> &layout::get_elements() {
        return m_elements;
    }

std::queue<std::unique_ptr<layout>> &layout::get_modals() {
        return m_modals;
    }
void layout::pop_modal()
{
    m_modals.pop();
}

void layout::get_requests(butil::draw_data &data)
{
    element::get_requests(data);
    // linear layouts get the draw call in addition order
    for (auto& elem : m_elements) {
        // TODO: final position should not be stored directly in the element and draw requests
        // shoulds be stored separately
        elem->set_position(elem->get_x()+get_x(), elem->get_y()+get_y());
        elem->get_requests(data);
    }
    if(!m_modals.empty())
        m_modals.front()->get_requests(data);
};