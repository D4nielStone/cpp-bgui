#include "elem/layout.hpp"
#include "bgui.hpp"

layout::layout() : element() {
    m_material.m_visible = false;
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
void layout::get_draw_requests(std::vector<butil::draw_request> &calls)
{
    static bool shader_compiled = false;
    if(!shader_compiled) {
        m_material.m_shader.compile("quad.vs", "quad.fs");
        shader_compiled = true;
    }
    element::get_draw_requests(calls);
    // linear layouts get the draw call in addition order
    for (auto& elem : m_elements) {
        elem->set_position(elem->get_x()+get_x(), elem->get_y()+get_y());
        elem->get_draw_requests(calls);
    }
    if(!m_modals.empty())
        m_modals.front()->get_draw_requests(calls);
};