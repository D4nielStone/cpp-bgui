#pragma once
#include "utils/style.hpp"
#include "utils/theme.hpp"

namespace bgui {
    inline void merge(computed_layout_style& out, const layout_style& in) {
        apply_optional(out.size_mode, in.size_mode);
        apply_optional(out.margin,    in.margin);
        apply_optional(out.padding,   in.padding);
        apply_optional(out.limit_min, in.limit_min);
        apply_optional(out.limit_max, in.limit_max);
        apply_optional(out.size,       in.size);
        apply_optional(out.align,      in.align);
        apply_optional(out.ori,      in.ori);
    }
    inline void merge(
        computed_visual_style& out,
        const visual_style& in,
        state state
    ) {
        out.background = in.background.resolve(state, out.background);
        out.border     = in.border.resolve(state, out.border);
        out.text       = in.text.resolve(state::normal, out.text);

        apply_optional(out.border_radius, in.border_radius);
        apply_optional(out.border_size, in.border_size);
        apply_optional(out.font, in.font);
        apply_optional(out.visible, in.visible);
    }
    inline void merge(
        visual_style& out,
        const visual_style& in
    ) {
        out.background.merge(in.background);
        out.border.merge(in.border);
        out.text.merge(in.text);
        merge_optional(out.border_radius, in.border_radius);
        merge_optional(out.border_size, in.border_size);
        merge_optional(out.font, in.font);
        merge_optional(out.visible, in.visible);
    }
    inline void merge(
        computed_style& out,
        const style& in,
        state state
    ) {
        merge(out.visual, in.visual, state);
        merge(out.layout, in.layout);
    }

    class style_manager {
    public:
        // Register
        void set_default(const style& s);
        void set_type(const std::string& type, const style& s);
        void set_class(const std::string& class_name, const style& s);
        void set_id(const std::string& id, const style& s);

        // Final Resolve
        void resolve(
            computed_style& cs,
            const std::string& type,
            const std::vector<std::string>& classes,
            const std::string& id,
            state state
        );  

        void apply_theme(const theme& t) {
            m_theme = t;
        }

        style& get_global() {
            return m_theme.base;
        }

        // singleton
        static style_manager& get_instance() {
            static style_manager sm;
            return sm;
        }
        style_manager(const style_manager&) = delete;
        style_manager& operator=(const style_manager&) = delete;
    private:
        style_manager() = default;
        ~style_manager() = default;

        theme m_theme;
    };
}