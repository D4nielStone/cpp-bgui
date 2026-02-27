#pragma once
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;
namespace bgui {
    using value = std::vector<std::string>; // value tokens
    using property_map = std::unordered_map<std::string, value>;

    struct style_block {
        property_map properties;
    };

    struct theme_data {
        style_block base;
        std::unordered_map<std::string, style_block> ids;
        std::unordered_map<std::string, style_block> types;
        std::unordered_map<std::string, style_block> classes;
    };

    fs::path resolve_path(const fs::path& input) {
        if (input.is_absolute())
            return input;

        return fs::absolute(input);
    }

    static inline std::string trim(std::string s) {
        auto is_space = [](char c) { return std::isspace(c); };
        s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), is_space));
        s.erase(std::find_if_not(s.rbegin(), s.rend(), is_space).base(), s.end());
        return s;
    }

    static inline std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> out;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim))
            out.push_back(trim(item));
        return out;
    }
    static theme_data parse_theme_stream(std::istream& in) {
        theme_data theme;

        enum class section_type {
            Base,
            Type,
            Class,
            Id,
            None
        };

        section_type current_section = section_type::None;
        style_block* current_block = nullptr;

        std::string line;

        while (std::getline(in, line)) {
            line = trim(line);

            // Ignore comments and empty lines
            if (line.empty() || line[0] == '#' || line[0] == ';')
                continue;

            // Section headers
            if (line.front() == '[' && line.back() == ']') {
                std::string header = line.substr(1, line.size() - 2);

                if (header == "base") {
                    current_section = section_type::Base;
                    current_block = &theme.base;
                }
                else if (header.rfind("type.", 0) == 0) {
                    std::string name = header.substr(5);
                    current_section = section_type::Type;
                    current_block = &theme.types[name];
                }
                else if (header.rfind("class.", 0) == 0) {
                    std::string name = header.substr(6);
                    current_section = section_type::Class;
                    current_block = &theme.classes[name];
                }
                else if (header.rfind("id.", 0) == 0) {
                    std::string name = header.substr(3);
                    current_section = section_type::Id;
                    current_block = &theme.ids[name];
                }
                else {
                    current_section = section_type::None;
                    current_block = nullptr;
                }

                continue;
            }

            auto eq = line.find('=');
            if (eq == std::string::npos || !current_block)
                continue;

            std::string key = trim(line.substr(0, eq));
            std::string value_str = trim(line.substr(eq + 1));

            value values = split(value_str, ',');
            current_block->properties[key] = values;
        }

        return theme;
    }
    theme_data parse_theme(const std::string& filename) {
        fs::path theme_path = resolve_path(filename);
        std::ifstream file(theme_path.c_str());

        if (!file) {
            std::cerr << "[Theme] Could not find the theme file "
                    << theme_path.string() << ".\n";
            return {};
        }

        return parse_theme_stream(file);
    }
}
