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

        if (v.size() < 3 && v.size() != 1)
            std::cerr << "[ThemeConverter] Invalid color value.\n";

        if(v.size()==1) 
            return bgui::color({
                to_float(v[0])
            });
        else
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
    static bgui::alignment to_alignment(const std::string& s) {
        if (s == "start")  return bgui::alignment::start;
        if (s == "center")   return bgui::alignment::center;
        if (s == "end") return bgui::alignment::end;

        return bgui::alignment::start;
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

        auto apply_key_color = [](bgui::state_color& c, const bgui::state& s, const bgui::value& val) {
            switch (s) {
                case state::normal:
                    c.normal = to_color(val);
                break;
                case state::hover:
                    c.hover = to_color(val);
                break;
                case state::pressed:
                    c.pressed = to_color(val);
                break;
                case state::focused:
                    c.focused = to_color(val);
                break;
                case state::disabled:
                    c.disabled = to_color(val);
                break;
            };
        };

        for (auto& [key, val] : block.properties) {

            // ---------------- VISUAL ----------------
            if (key.starts_with("visual.background.")) {
                auto state = to_state(key.substr(18));
                apply_key_color(out.visual.background, state, val);
            }
            else if (key.starts_with("visual.border.")) {
                auto state = to_state(key.substr(14));
                apply_key_color(out.visual.border, state, val);
            }
            else if (key.starts_with("visual.text.")) {
                auto state = to_state(key.substr(12));
                apply_key_color(out.visual.text, state, val);
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
            else if (key == "visual.font") {
                out.visual.font = val[0];
            }

            // ---------------- LAYOUT ----------------
            else if (key == "layout.mode") {
                mode w = to_mode(val[0]);
                mode h = val.size() > 1
                    ? to_mode(val[1])
                    : w;
                out.layout.size_mode = {w, h};
            }
            else if (key == "layout.align") {
                alignment a = to_alignment(val[0]);
                alignment b = val.size() > 1
                    ? to_alignment(val[1])
                    : b;
                out.layout.align = {a, b};
            }
            else if (key == "layout.padding") {
                if (val.size() == 1)
                    out.layout.padding = {to_int(val[0]), to_int(val[0]), to_int(val[0]), to_int(val[0])};
                else if(val.size() == 2)
                    out.layout.padding = {to_int(val[0]), to_int(val[1]), to_int(val[0]), to_int(val[1])};
                else if (val.size() == 4)
                    out.layout.padding = {to_int(val[0]), to_int(val[1]), to_int(val[2]), to_int(val[3])};
                else
                    std::cerr << "[ThemeConverter] Invalid padding value.\n";
            }
            else if (key == "layout.margin") {
                if (val.size() == 1)
                    out.layout.margin = {to_int(val[0]), to_int(val[0]), to_int(val[0]), to_int(val[0])};
                else if(val.size() == 2)
                    out.layout.margin = {to_int(val[0]), to_int(val[1]), to_int(val[0]), to_int(val[1])};
                else if (val.size() == 4)
                    out.layout.margin = {to_int(val[0]), to_int(val[1]), to_int(val[2]), to_int(val[3])};
                else
                    std::cerr << "[ThemeConverter] Invalid margin value.\n";
            }
            else if (key == "layout.size") {
                if (val.size() == 1)
                    out.layout.size = {to_float(val[0]), to_float(val[0])};
                else if(val.size() == 2)
                    out.layout.size = {to_float(val[0]), to_float(val[1])};
                else
                    std::cerr << "[ThemeConverter] Invalid size value.\n";
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

        // Ids
        for (auto& [name, block] : data.ids)
            out.ids[name] = convert_block(block);

        return out;
    }
}