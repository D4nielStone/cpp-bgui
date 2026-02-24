#include "os/font.hpp"
#include <stdexcept>
#include <filesystem>
#include <iostream>
    
static bgui::font_manager instance;

bgui::font_manager & bgui::font_manager::get_font_manager() {
    return instance;
}
bgui::font_manager::font_manager() {

    m_fonts.clear();
}

bgui::font_manager::~font_manager() {
    m_fonts.clear();
}

bool bgui::font_manager::has_font(const std::string &name) {
    return m_fonts.find(name) != m_fonts.end();
}
bgui::font &bgui::font_manager::get_font(const std::string& name) {
    if(name == "default") {
        if(!m_fonts.empty())
            return m_fonts.begin()->second;
        else throw std::runtime_error("Font " + name + " not found. Have you added it to the backend?");
    }
    if(has_font(name))
        return m_fonts[name];
    else throw std::runtime_error("Font " + name + " not found. Have you added it to the backend?");
}
