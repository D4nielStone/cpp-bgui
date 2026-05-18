#pragma once
#include <unordered_map>
#include "style.hpp"

namespace bgui {

    struct theme {
        style base;
        std::unordered_map<std::string, style> types;
        std::unordered_map<std::string, style> classes;
        std::unordered_map<std::string, style> ids;
    };

    bgui::theme dark_theme();
    bgui::theme light_theme();
}