#pragma once
#include "element.hpp"
#include "utils/draw.hpp"
#include "utils/style.hpp"
#include "os/font.hpp"
#include "utils/syntax_highlight.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace bgui {
    class text : public element {
    private:
        std::string m_buffer, m_last_font;
        float m_scale;
        size_t m_cursor_position = 0;
        bool m_cursor_visible = false;
        size_t m_selection_start = 0;
        size_t m_selection_end = 0;
        bool m_wrap_enabled = true;
        bool m_editor_guides_enabled = false;
        std::shared_ptr<const std::vector<syntax_highlight_rule>> m_highlight_rules;
        std::vector<std::optional<bgui::vec4>> m_highlight_colors;
        std::vector<std::optional<size_t>> m_brace_pairs;
        std::vector<std::pair<size_t, size_t>> m_scope_pairs;

        void update_highlight_colors();
        float get_line_number_gutter_width(const font& font, float scale) const;
    public:
        text(const std::string& buffer, float scale);
        ~text();
        // sets to this text the font with `name`.
        void set_font(const std::string& name);
        void set_wrap(bool enabled) {
            if (m_wrap_enabled == enabled)
                return;
            m_wrap_enabled = enabled;
            mark_layout_dirty();
        }
        bool is_wrap_enabled() const { return m_wrap_enabled; }
        void set_editor_guides_enabled(bool enabled);
        void on_update() override;
        void calc_content_size(const layer& lay) override;
        float get_text_width(const std::string& t) const;
        void set_cursor(size_t position, bool visible) {
            if (m_cursor_position == position && m_cursor_visible == visible)
                return;
            m_cursor_position = position;
            m_cursor_visible = visible;
            mark_render_dirty();
        }
        void set_selection(size_t start, size_t end) {
            const auto selection_start = std::min(start, end);
            const auto selection_end = std::max(start, end);
            if (m_selection_start == selection_start &&
                m_selection_end == selection_end)
                return;
            m_selection_start = selection_start;
            m_selection_end = selection_end;
            mark_render_dirty();
        }
        void set_buffer(const std::string& buffer);
        void set_highlight_rules(
            std::shared_ptr<const std::vector<syntax_highlight_rule>> rules
        );
        size_t get_cursor_position_at(float x, float y) const;
        size_t get_vertical_cursor_position(size_t position, int direction) const;
        const std::string& get_buffer() const { return m_buffer; };
        void get_requires(bgui::draw_data *calls) override;
    };
} // namespace bgui