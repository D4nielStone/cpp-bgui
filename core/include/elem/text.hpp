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
        friend class inputbox;
        float m_horizontal_scroll = 0.f;
        void reveal_cursor_horizontally(float left, float right);
        std::string m_buffer, m_last_font;
        float m_scale;
        bgui::vec2 m_draw_offset{0.f};
        size_t m_cursor_position = 0;
        bool m_cursor_visible = false;
        size_t m_selection_start = 0;
        size_t m_selection_end = 0;
        bool m_wrap_enabled = true;
        std::optional<float> m_wrap_width;
        bool m_editor_guides_enabled = false;
        bool m_line_numbers_enabled = false;
        struct text_chunk { size_t codepoint; size_t byte; size_t line; };
        std::vector<text_chunk> m_chunks;
        std::u32string m_codepoints;
        std::vector<size_t> m_byte_lengths;
        size_t m_line_count = 1;
        bool m_metrics_valid = false;
        const font* m_metrics_font = nullptr;
        float m_metrics_scale = 0.f;
        float m_metrics_wrap_width = 0.f;
        float wrap_width() const;
        bool m_metrics_wrap = false;
        bool m_metrics_numbers = false;
        vec2i m_metrics_size{0};
        vec2 m_metrics_offset{0.f};
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
        // Logical pixels; nullopt restores the default wrapping policy.
        void set_wrap_width(std::optional<float> width) {
            if (width) width = std::max(1.f, *width);
            if (m_wrap_width == width) return;
            m_wrap_width = width;
            m_metrics_valid = false;
            mark_layout_dirty();
        }
        void set_editor_guides_enabled(bool enabled);
        void set_line_numbers_enabled(bool enabled) {
            if (m_line_numbers_enabled == enabled) return;
            m_line_numbers_enabled = enabled;
            m_metrics_valid = false;
            mark_layout_dirty();
        }
        bool line_numbers_enabled() const { return m_line_numbers_enabled; }
        size_t chunk_count() const { return m_chunks.size(); }
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
