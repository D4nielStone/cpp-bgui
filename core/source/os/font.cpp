#include "os/font.hpp"
#include "os/asset_manager.hpp"
#include "utils/logging.hpp"
#include <stdexcept>
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

    bgui::asset_manager::get_instance().set_default_font(name, resolution);
}

bool bgui::font_manager::has_font(
    const std::string& name,
    const unsigned int resolution
) const {
    return bgui::asset_manager::get_instance().has_font(name, resolution);
}

bgui::font& bgui::font_manager::get_font(
    const std::string& name,
    const unsigned int resolution
) {
    auto& manager = bgui::asset_manager::get_instance();
    if (manager.has_font(name, resolution))
        return manager.get_font(name, resolution);

    bgui::detail::log_err() << "[FONT] Font " << name << " not found. Have you added it to the backend?" << std::endl;
    return manager.get_font(name, resolution);
}
