#pragma once
#include "material.hpp"
#include "enums.hpp"
#include "vec.hpp"
#include <optional>
#include <climits>

namespace bgui {

    template<typename T>
    inline void apply_optional(T& dst, const std::optional<T>& src) {
        if (src.has_value()) dst = *src;
    }
    template<typename T>
    inline void merge_optional(std::optional<T>& dst, const std::optional<T>& src) {
        if(dst) return;
        else if (src) dst = src;
    }
    struct layout_style {
        std::optional<vec<2, mode>> size_mode;
        std::optional<vec4i> margin;
        std::optional<vec4i> padding;
        std::optional<vec2i> limit_min;
        std::optional<vec2i> limit_max;
        std::optional<vec2> size;
        std::optional<alignment> align;
        std::optional<alignment> cross_align;
        std::optional<orientation> ori;


        void set_padding(int a, int b) {
            padding = vec4i{a, b, a, b};
        }
        void set_margin(int a, int b) {
            margin = vec4i{a, b, a, b};
        }

        void require_height(mode m, float v = 100.f) {
            if(!size_mode) size_mode = {mode::pixel};
            if(!size) size = {100.f};
            (*size_mode)[1] = m;
            (*size)[1] = v;
        }
        void require_width(mode m, float v = 100.f) {
            if(!size_mode) size_mode = {mode::pixel};
            if(!size) size = {100.f};
            (*size_mode)[0] = m;
            (*size)[0] = v;
        }
        void require_size(float a, float b) {
            if(!size_mode) size_mode = {mode::pixel};
            if(!size) size = {100.f};
            size = {a, b};
        }void require_mode(mode a, mode b) {
            if(!size_mode) size_mode = {mode::pixel};
            if(!size) size = {100.f};
            size_mode = {a, b};
        }
    };
    
    struct state_color {
        std::optional<color> normal;
        std::optional<color> hover;
        std::optional<color> pressed;
        std::optional<color> focused;
        std::optional<color> disabled;

        color resolve(state state, const color& fallback) const {
            if (state == state::hover    && hover)    return *hover;
            if (state == state::pressed  && pressed)  return *pressed;
            if (state == state::focused  && focused)  return *focused;
            if (state == state::disabled && disabled) return *disabled;
            if (normal) return *normal;
            return fallback;
        }
        void merge(const state_color& other) {
            merge_optional(normal, other.normal);
            merge_optional(hover, other.hover);
            merge_optional(pressed, other.pressed);
            merge_optional(focused, other.focused);
            merge_optional(disabled, other.disabled);
        }
    };

    struct visual_style {
        state_color background;
        state_color border;
        state_color text;

        std::optional<float> border_radius, border_size;
        std::optional<std::string> font;
        std::optional<bool> visible;
    };

    // \brief Style structure that contains visual and layout styles
    struct style {
        // Layout style parameters
        layout_style layout;
        // Visual style parameters
        visual_style visual;
    };
    struct computed_layout_style {
        vec<2, mode> size_mode {mode::pixel, mode::pixel};
        vec2 size {0.f, 0.f};
        vec2i limit_min {15, 15};
        vec2i limit_max {INT_MAX, INT_MAX};
        vec4i padding {0, 0};
        vec4i margin {0, 0};
        alignment align {alignment::start};
        alignment cross_align {alignment::start};
        orientation ori {orientation::horizontal};
    };

    struct computed_visual_style {
        color background {0.f, 0.f, 0.f, 0.f};
        color border {1.f, 1.f, 1.f, 1.f};
        color text {0.f, 0.f, 0.f, 0.f};

        std::string font = "default";
        float border_radius {2.f}, border_size{2.f};
        bool visible {true};
    };

    struct computed_style {
        computed_layout_style layout;
        computed_visual_style visual;
    };

} // namespace bgui