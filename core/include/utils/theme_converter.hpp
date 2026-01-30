#pragma once
#include "theme_parser.hpp"
#include "vec.hpp"
#include "style.hpp"
#include "enums.hpp"
#include "theme.hpp"

namespace bgui {
    static bool to_bool(const std::string& v) {
        return v == "true" || v == "1";
    }
    static float to_float(const std::string& v) {
        return std::stof(v);
    }

    static int to_int(const std::string& v) {
        return std::stoi(v);
    }
    static bgui::color to_color(const bgui::value& v) {
        if (v.size() == 1 && v[0] == "0.0")
            return {0,0,0,0};

        if (v.size() < 3)
            throw std::runtime_error("Invalid color value");

        return {
            to_float(v[0]),
            to_float(v[1]),
            to_float(v[2]),
            v.size() >= 4 ? to_float(v[3]) : 1.f
        };
    }
    static bgui::state to_state(const std::string& s) {
        if (s == "normal")  return bgui::state::normal;
        if (s == "hover")   return bgui::state::hover;
        if (s == "pressed") return bgui::state::pressed;

        return bgui::state::normal;
    }
    static bgui::mode to_mode(const std::string& s) {
        if (s == "stretch")        return bgui::mode::stretch;
        if (s == "wrap_content")  return bgui::mode::wrap_content;
        if (s == "match_parent")  return bgui::mode::match_parent;
        if (s == "same")          return bgui::mode::same;

        return bgui::mode::pixel;
    }
    static bgui::style convert_block(const bgui::style_block& block) {
        bgui::style out;

        for (auto& [key, val] : block.properties) {

            // ---------------- VISUAL ----------------
            if (key.starts_with("visual.background.")) {
                auto state = to_state(key.substr(18));
                switch (state) {
                    case state::normal:
                        out.visual.background.normal = to_color(val);
                    break;
                    case state::hover:
                        out.visual.background.hover = to_color(val);
                    break;
                    case state::pressed:
                        out.visual.background.pressed = to_color(val);
                    break;
                    case state::focused:
                        out.visual.background.focused = to_color(val);
                    break;
                    case state::disabled:
                        out.visual.background.disabled = to_color(val);
                    break;
                };
            }
            else if (key.starts_with("visual.border.")) {
                auto state = to_state(key.substr(14));
            }
            else if (key.starts_with("visual.text.")) {
                auto state = to_state(key.substr(12));
            }
            else if (key == "visual.visible") {
                out.visual.visible = to_bool(val[0]);
            }
            else if (key == "visual.border.radius") {
                out.visual.border_radius = to_float(val[0]);
            }
            else if (key == "visual.border.size") {
                out.visual.border_size = to_float(val[0]);
            }

            // ---------------- LAYOUT ----------------
            else if (key == "layout.mode") {
                mode w = to_mode(val[0]);
                mode h = val.size() > 1
                    ? to_mode(val[1])
                    : w;
                out.layout.size_mode = {w, h};
            }
            else if (key == "layout.padding") {
                out.layout.padding = {to_int(val[0]), to_int(val[1]), to_int(val[2]), to_int(val[3])};
            }
            else if (key == "layout.margin") {
                out.layout.margin = {to_int(val[0]), to_int(val[1]), to_int(val[2]), to_int(val[3])};
            }
        }

        return out;
    }
    bgui::theme convert_theme(const bgui::theme_data& data) {
        bgui::theme out;

        // Base
        out.base = convert_block(data.base);

        // Types
        for (auto& [name, block] : data.types)
            out.types[name] = convert_block(block);

        // Classes
        for (auto& [name, block] : data.classes)
            out.classes[name] = convert_block(block);

        // Ids (se você ativar depois no parser)
        for (auto& [name, block] : data.ids)
            out.ids[name] = convert_block(block);

        return out;
    }
}