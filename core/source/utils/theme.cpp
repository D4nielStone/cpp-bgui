#include "utils/theme.hpp"
#include "utils/theme_parser.hpp"
#include "utils/theme_converter.hpp"

bgui::theme bgui::dark_theme() {
    auto parsed = bgui::parse_theme("assets/themes/dark_theme.ini");
    auto t = bgui::convert_theme(parsed);

    return t;
}