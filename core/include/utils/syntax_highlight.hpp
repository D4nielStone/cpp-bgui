#pragma once

#include <regex>
#include <string>
#include <vector>

#include "utils/vec.hpp"

namespace bgui {
    struct syntax_highlight_rule {
        std::string pattern;
        std::regex expression;
        bgui::vec4 color;
    };

    std::vector<syntax_highlight_rule> load_syntax_highlight_config(const std::string& path);
}
