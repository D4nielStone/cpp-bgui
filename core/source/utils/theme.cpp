#include "utils/theme.hpp"
#include "utils/theme_parser.hpp"
#include "utils/theme_converter.hpp"
#include <array>
#include <filesystem>
#include <stdexcept>

namespace {
    bgui::theme load_theme_from_candidates(const std::string& filename) {
        const std::array<std::filesystem::path, 3> candidates = {
            std::filesystem::path("assets") / filename,
            std::filesystem::path("../assets") / filename,
#ifdef BGUI_ASSETS_DIR
            std::filesystem::path(BGUI_ASSETS_DIR) / filename
#else
            std::filesystem::path()
#endif
        };

        for (const auto& candidate : candidates) {
            if (candidate.empty() || !std::filesystem::is_regular_file(candidate))
                continue;

            return bgui::convert_theme(bgui::parse_theme(candidate.string()));
        }

        throw std::runtime_error("[Theme] Could not find asset: " + filename);
    }
}

bgui::theme bgui::load_theme(const std::string& filename) {
    return bgui::convert_theme(bgui::parse_theme(filename));
}

bgui::theme bgui::dark_theme() {
    return load_theme_from_candidates("themes/dark.theme");
}

bgui::theme bgui::light_theme() {
    return load_theme_from_candidates("themes/light.theme");
}
