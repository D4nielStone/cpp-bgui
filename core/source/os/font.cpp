#include "os/font.hpp"
#include <stdexcept>
#include <iostream>
#include <limits>

const char* bgui::font_style_name(const bgui::font_style style) {
    switch (style) {
        case bgui::font_style::regular: return "regular";
        case bgui::font_style::bold: return "bold";
        case bgui::font_style::italic: return "italic";
        case bgui::font_style::bold_italic: return "bold_italic";
        case bgui::font_style::light: return "light";
        case bgui::font_style::semibold: return "semibold";
        case bgui::font_style::black: return "black";
        case bgui::font_style::thin: return "thin";
    }
    return "regular";
}

bgui::font_manager& bgui::font_manager::get_instance() {
    static bgui::font_manager instance;
    return instance;
}

bgui::font_manager::font_manager() = default;

std::string bgui::font_manager::make_key(
    const std::string& name,
    const unsigned int resolution
) {
    return name + "#" + std::to_string(resolution);
}

void bgui::font_manager::set_font_loaded_callback(
    std::function<void(const bgui::font&)> callback
) {
    m_on_font_loaded = std::move(callback);
}

void bgui::font_manager::set_default_font(
    const std::string& name,
    const unsigned int resolution
) {
    if (name.empty() || resolution == 0)
        throw std::invalid_argument("Default font name and resolution are required.");

    const auto key = make_key(name, resolution);
    if (m_fonts.find(key) == m_fonts.end())
        throw std::runtime_error("Cannot set an unloaded font as the default.");

    m_default_font_keys[resolution] = key;
}

bool bgui::font_manager::has_font(
    const std::string& name,
    const unsigned int resolution
) const {
    if (name == "default") {
        const auto default_it = m_default_font_keys.find(resolution);
        return default_it != m_default_font_keys.end() &&
            m_fonts.find(default_it->second) != m_fonts.end();
    }

    return m_fonts.find(make_key(name, resolution)) != m_fonts.end();
}

bgui::font& bgui::font_manager::get_font(
    const std::string& name,
    const unsigned int resolution
) {
    if (resolution == 0)
        throw std::invalid_argument("Font resolution must be greater than zero.");

    if(name == "default") {
        const auto default_key = m_default_font_keys.find(resolution);
        if (default_key != m_default_font_keys.end()) {
            const auto default_font = m_fonts.find(default_key->second);
            if (default_font != m_fonts.end())
                return default_font->second;
        }
    }

    const auto it = m_fonts.find(make_key(name, resolution));
    if (it != m_fonts.end())
        return it->second;

    std::cerr << "[FONT] Font " << name << " not found. Have you added it to the backend?" << std::endl;
    throw std::runtime_error(
        "Font " + name + " at resolution " +
        std::to_string(resolution) + " was not found."
    );
}
