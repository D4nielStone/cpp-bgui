
#include "lay/relative.hpp"
#include "bgui.hpp"
using namespace blay;

using namespace butil;
    
relative::relative(const orientation& ori) {
    m_orientation = ori;
    m_visible = false;
    m_material.m_shader_tag = "ui::default";
}

void relative::update() {
}

void relative::fit_to_content() {
}
