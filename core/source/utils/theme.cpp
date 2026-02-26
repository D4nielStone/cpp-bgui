#include "utils/theme.hpp"
#include "utils/theme_parser.hpp"
#include "utils/theme_converter.hpp"

std::string dtstr = R"(
# =========================
# BASE (default)
# =========================
[base]
visual.background.normal = 0
visual.text.normal       = 1.0, 1.0, 1.0, 1.0
visual.font              = default
visual.border.normal     = 0.04, 0.04, 0.04, 1.0


# =========================
# TYPES
# =========================

[type.linear]
visual.background.normal = 0.08, 0.08, 0.08, 1.0
visual.border.normal     = 0.10, 0.10, 0.10, 1.0
visual.visible           = false
layout.padding           = 5, 2


[type.window]
visual.background.normal = 0.08, 0.08, 0.08, 1.0
visual.border.normal     = 0.10, 0.10, 0.10, 1.0
visual.visible           = true
layout.padding           = 0, 0
layout.size              = 400, 300


[type.element]
visual.background.normal = 0.08, 0.08, 0.08, 1.0


# =========================
# BUTTON
# =========================

[type.button]
visual.background.normal  = 0.10, 0.10, 0.10, 1.0
visual.background.hover   = 0.12, 0.12, 0.12, 1.0
visual.background.pressed = 0.12, 0.12, 0.12, 1.0
visual.border.normal      = 0.08, 0.08, 0.08, 1.0
visual.border.radius      = 10
visual.border.size        = 1
layout.padding            = 5, 5, 5, 5
layout.mode               = stretch, wrap_content


# =========================
# INPUT AREA
# =========================

[type.inputarea]
visual.background.normal  = 0.10, 0.10, 0.10, 1.0
visual.background.hover   = 0.12, 0.12, 0.12, 1.0
visual.background.pressed = 0.12, 0.12, 0.12, 1.0
visual.border.normal      = 0.04, 0.04, 0.04, 1.0
visual.border.radius      = 4
visual.border.size        = 1
layout.padding            = 5, 5, 5, 5
layout.mode               = stretch, wrap_content


# =========================
# CHECKBOX
# =========================

[type.checkbox]
visual.background.normal = 0.10, 0.10, 0.10, 1.0
visual.background.hover  = 0.12, 0.12, 0.12, 1.0
visual.border.normal     = 0.08, 0.08, 0.08, 1.0
visual.border.radius     = 10
visual.border.size       = 1
layout.padding           = 2, 2
layout.mode              = stretch, wrap_content


# =========================
# TEXT
# =========================

[type.text]
visual.background.normal = 0.0
visual.border.normal     = 0.0
visual.text.normal       = 1.0, 1.0, 1.0, 1.0
visual.font              = Ubuntu Regular
layout.mode              = wrap_content, wrap_content
layout.align             = center


# =========================
# CLASSES
# =========================

[class.window-header]
layout.mode               = stretch, wrap_content
layout.padding            = 0, 0
visual.background.normal  = 0.07, 0.07, 0.07, 1.0
visual.border.normal      = 0.0
visual.visible            = true


[class.window-button]
layout.mode              = wrap_content, wrap_content
layout.padding           = 0
layout.margin            = 0
visual.background.normal = 0.0
visual.border.normal     = 0.0


[class.inputarea-txt]
layout.mode = stretch, wrap_content


[class.checkbox-box]
visual.background.pressed = 0.5, 0.5, 0.5, 1.0
visual.border.normal      = 0.08, 0.08, 0.08, 1.0
visual.border.radius      = 10
visual.border.size        = 2
layout.margin             = 5, 5
layout.limit_min          = 12, 12
layout.mode               = same, match_parent


[class.checkbox-txt]
layout.mode   = stretch, wrap_content
layout.align  = end
layout.margin = 1, 1


[class.window-label]
visual.background.normal = 0.0
visual.border.normal     = 0.0
layout.align             = center
layout.mode              = stretch, wrap_content


# =========================
# ACCENT CLASSES
# =========================

[class.primary]
visual.background.normal = 0.2, 0.4, 0.8, 1.0
visual.background.hover  = 0.3, 0.5, 0.9, 1.0


[class.danger]
visual.background.normal = 0.8, 0.2, 0.2, 1.0
visual.background.hover  = 0.9, 0.3, 0.3, 1.0)";
bgui::theme bgui::dark_theme() {
    std::istringstream s(dtstr);
    auto parsed = bgui::parse_theme_stream(s);
    auto t = bgui::convert_theme(parsed);

    return t;
}