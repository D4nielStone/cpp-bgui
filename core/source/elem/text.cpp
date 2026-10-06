#include "elem/text.hpp"
#include <bgui.hpp>
#include <iostream>
#include <codecvt>
#include <locale>
#include <algorithm>
#include <array>
#include <vector>

// Converte UTF-8 -> UTF-32 e também devolve, para cada codepoint,
// quantos bytes ele ocupava na string original. Isso é essencial
// para não misturar "índice de codepoint" com "offset em bytes"
// ao usar std::string::substr().
std::u32string utf8_to_utf32(const std::string& str, std::vector<size_t>* byte_lengths = nullptr) {
    std::u32string result;
    size_t i = 0;
    while (i < str.length()) {
        unsigned char c = str[i];
        char32_t ch = 0;
        size_t len = 1;

        if ((c & 0x80) == 0) {
            ch = c;
            len = 1;
        } else if ((c & 0xE0) == 0xC0) {
            ch = ((c & 0x1F) << 6) | (str[i + 1] & 0x3F);
            len = 2;
        } else if ((c & 0xF0) == 0xE0) {
            ch = ((c & 0x0F) << 12) | ((str[i + 1] & 0x3F) << 6) | (str[i + 2] & 0x3F);
            len = 3;
        } else if ((c & 0xF8) == 0xF0) {
            ch = ((c & 0x07) << 18) | ((str[i + 1] & 0x3F) << 12) | ((str[i + 2] & 0x3F) << 6) | (str[i + 3] & 0x3F);
            len = 4;
        }

        result.push_back(ch);
        if (byte_lengths) byte_lengths->push_back(len);
        i += len;
    }
    return result;
}

namespace {
    struct cursor_stop {
        size_t position;
        float x;
    };

    float glyph_advance(char32_t character, const bgui::font& font, float scale) {
        if (character == U'\t') {
            const auto space = font.chs.find(U' ');
            return space == font.chs.end() ? 0.f : space->second.advance * scale * 4.f;
        }
        const auto glyph = font.chs.find(character);
        return glyph == font.chs.end() ? 0.f : glyph->second.advance * scale;
    }

    std::vector<bool> find_lua_non_code(const std::u32string& codepoints) {
        std::vector<bool> non_code(codepoints.size(), false);
        auto long_bracket_end = [&](size_t start) -> std::optional<size_t> {
            if (start >= codepoints.size() || codepoints[start] != U'[')
                return std::nullopt;
            size_t opening_end = start + 1;
            while (opening_end < codepoints.size() && codepoints[opening_end] == U'=')
                ++opening_end;
            if (opening_end >= codepoints.size() || codepoints[opening_end] != U'[')
                return std::nullopt;

            const size_t equals_count = opening_end - start - 1;
            for (size_t index = opening_end + 1; index < codepoints.size(); ++index) {
                if (codepoints[index] != U']')
                    continue;
                size_t closing_end = index + 1;
                size_t matched_equals = 0;
                while (closing_end < codepoints.size() &&
                       matched_equals < equals_count &&
                       codepoints[closing_end] == U'=') {
                    ++closing_end;
                    ++matched_equals;
                }
                if (matched_equals == equals_count &&
                    closing_end < codepoints.size() &&
                    codepoints[closing_end] == U']')
                    return closing_end + 1;
            }
            return std::nullopt;
        };

        for (size_t index = 0; index < codepoints.size();) {
            if (codepoints[index] == U'\'' || codepoints[index] == U'"') {
                const char32_t quote = codepoints[index];
                non_code[index++] = true;
                while (index < codepoints.size()) {
                    non_code[index] = true;
                    if (codepoints[index] == U'\\') {
                        ++index;
                        if (index < codepoints.size())
                            non_code[index++] = true;
                        continue;
                    }
                    if (codepoints[index++] == quote)
                        break;
                }
                continue;
            }

            if (codepoints[index] == U'-' && index + 1 < codepoints.size() &&
                codepoints[index + 1] == U'-') {
                const size_t comment_start = index;
                index += 2;
                const auto long_comment_end = long_bracket_end(index);
                if (long_comment_end) {
                    index = *long_comment_end;
                } else {
                    while (index < codepoints.size() && codepoints[index] != U'\n')
                        ++index;
                }
                std::fill(non_code.begin() + static_cast<std::ptrdiff_t>(comment_start),
                          non_code.begin() + static_cast<std::ptrdiff_t>(index), true);
                continue;
            }

            const auto long_string_end = long_bracket_end(index);
            if (long_string_end) {
                std::fill(non_code.begin() + static_cast<std::ptrdiff_t>(index),
                          non_code.begin() + static_cast<std::ptrdiff_t>(*long_string_end), true);
                index = *long_string_end;
                continue;
            }
            ++index;
        }
        return non_code;
    }

    std::vector<std::vector<cursor_stop>> build_cursor_rows(
        const std::string& buffer,
        const bgui::font& font,
        float scale,
        bool wrap_enabled,
        float wrap_width = 500.f
    ) {
        std::vector<std::vector<cursor_stop>> rows(1);
        rows.front().push_back({0, 0.f});

        std::vector<size_t> byte_lengths;
        const auto codepoints = utf8_to_utf32(buffer, &byte_lengths);
        size_t byte_position = 0;
        float line_x = 0.f;

        for (size_t index = 0; index < codepoints.size(); ++index) {
            const char32_t character = codepoints[index];
            if (character == U'\n') {
                byte_position += byte_lengths[index];
                rows.emplace_back();
                rows.back().push_back({byte_position, 0.f});
                line_x = 0.f;
                continue;
            }

            const float advance = glyph_advance(character, font, scale);
            if (wrap_enabled && line_x + advance > wrap_width && line_x > 0.f) {
                rows.emplace_back();
                rows.back().push_back({byte_position, 0.f});
                line_x = 0.f;
            }

            line_x += advance;
            byte_position += byte_lengths[index];
            rows.back().push_back({byte_position, line_x});
        }
        return rows;
    }
}

bgui::text::text(const std::string &buffer, float scale) : m_buffer(buffer), m_scale(scale) {
    type = "text";
    set_font(computed_style.visual.font);
    m_material.m_use_tex = true;
    m_material.m_shader_tag = "ui::text";
    update_highlight_colors();
}
bgui::text::~text() {
}
void bgui::text::set_buffer(const std::string& buffer) {
    if (m_buffer == buffer)
        return;
    m_buffer = buffer;
    update_highlight_colors();
}

void bgui::text::set_highlight_rules(
    std::shared_ptr<const std::vector<syntax_highlight_rule>> rules
) {
    m_highlight_rules = std::move(rules);
    update_highlight_colors();
}

void bgui::text::set_editor_guides_enabled(bool enabled) {
    if (m_editor_guides_enabled == enabled)
        return;
    m_editor_guides_enabled = enabled;
    update_highlight_colors();
}

size_t bgui::text::get_cursor_position_at(float x, float y) const {
    auto& font_manager = bgui::font_manager::get_instance();
    if (!font_manager.has_font(computed_style.visual.font))
        return 0;

    const auto& font = font_manager.get_font(computed_style.visual.font);
    const float scale = m_scale * bgui::get_global_scale();
    const float line_height = (font.ascent + font.descent + font.line_gap) * scale;
    if (line_height <= 0.f)
        return 0;

    const auto rows = build_cursor_rows(m_buffer, font, scale, m_wrap_enabled);
    const float row_position = (y - processed_y()) / line_height;
    const size_t row_index = static_cast<size_t>(std::clamp(
        static_cast<int>(std::floor(row_position)), 0,
        static_cast<int>(rows.size() - 1)));

    const int total_width = static_cast<int>(get_text_width(m_buffer));
    const float gutter_width = get_line_number_gutter_width(font, scale);
    const int combined_width = total_width + static_cast<int>(gutter_width);
    float origin_x = static_cast<float>(processed_x());
    switch (computed_style.layout.align.x) {
        case bgui::alignment::center:
            origin_x += (processed_width() - combined_width) / 2 + gutter_width;
            break;
        case bgui::alignment::end:
            origin_x += processed_width() - combined_width + gutter_width;
            break;
        case bgui::alignment::start:
            origin_x += gutter_width;
            break;
    }

    const float local_x = x - origin_x;
    const auto& stops = rows[row_index];
    size_t closest_position = stops.front().position;
    float closest_distance = std::abs(local_x - stops.front().x);
    for (const auto& stop : stops) {
        const float distance = std::abs(local_x - stop.x);
        if (distance < closest_distance) {
            closest_position = stop.position;
            closest_distance = distance;
        }
    }
    return closest_position;
}

float bgui::text::get_line_number_gutter_width(const font& font, float scale) const {
    if (!m_editor_guides_enabled)
        return 0.f;

    const size_t line_count = static_cast<size_t>(
        std::count(m_buffer.begin(), m_buffer.end(), '\n')) + 1;
    const size_t digit_count = std::to_string(line_count).size();
    float widest_digit = 0.f;
    for (char32_t digit = U'0'; digit <= U'9'; ++digit) {
        const auto glyph = font.chs.find(digit);
        if (glyph != font.chs.end())
            widest_digit = std::max(widest_digit, glyph->second.advance * scale);
    }
    const float padding = 5.f * bgui::get_global_scale();
    return widest_digit * static_cast<float>(digit_count) + padding * 2.f;
}

size_t bgui::text::get_vertical_cursor_position(size_t position, int direction) const {
    auto& font_manager = bgui::font_manager::get_instance();
    if (direction == 0 || !font_manager.has_font(computed_style.visual.font))
        return position;

    const auto& font = font_manager.get_font(computed_style.visual.font);
    const auto rows = build_cursor_rows(
        m_buffer, font, m_scale * bgui::get_global_scale(), m_wrap_enabled);
    size_t current_row = rows.size();
    float current_x = 0.f;

    for (size_t row = 0; row < rows.size(); ++row) {
        for (const auto& stop : rows[row]) {
            if (stop.position == position) {
                current_row = row;
                current_x = stop.x;
            }
        }
    }
    if (current_row == rows.size())
        return position;

    const size_t target_row = direction < 0
        ? (current_row == 0 ? 0 : current_row - 1)
        : std::min(current_row + 1, rows.size() - 1);
    const auto& stops = rows[target_row];
    size_t closest_position = stops.front().position;
    float closest_distance = std::abs(current_x - stops.front().x);
    for (const auto& stop : stops) {
        const float distance = std::abs(current_x - stop.x);
        if (distance < closest_distance) {
            closest_position = stop.position;
            closest_distance = distance;
        }
    }
    return closest_position;
}

void bgui::text::update_highlight_colors() {
    std::vector<size_t> byte_lengths;
    const std::u32string codepoints = utf8_to_utf32(m_buffer, &byte_lengths);
    m_highlight_colors.assign(codepoints.size(), std::nullopt);
    m_brace_pairs.assign(codepoints.size(), std::nullopt);
    m_scope_pairs.clear();

    if (m_highlight_rules && !m_highlight_rules->empty()) {
        std::vector<size_t> byte_positions(codepoints.size() + 1, 0);
        for (size_t index = 0; index < byte_lengths.size(); ++index)
            byte_positions[index + 1] = byte_positions[index] + byte_lengths[index];

        for (const auto& rule : *m_highlight_rules) {
            for (std::sregex_iterator match(m_buffer.begin(), m_buffer.end(), rule.expression),
                 end; match != end; ++match) {
                const size_t match_start = static_cast<size_t>(match->position());
                const size_t match_end = match_start + static_cast<size_t>(match->length());
                if (match_start == match_end)
                    continue;

                const auto first_boundary = std::upper_bound(
                    byte_positions.begin(), byte_positions.end(), match_start);
                size_t index = static_cast<size_t>(first_boundary - byte_positions.begin() - 1);
                for (; index < codepoints.size(); ++index) {
                    if (byte_positions[index] >= match_end)
                        break;
                    if (byte_positions[index + 1] > match_start && !m_highlight_colors[index])
                        m_highlight_colors[index] = rule.color;
                }
            }
        }
    }

    static const std::array<bgui::vec4, 3> brace_colors{{
        {1.f, 0.843f, 0.f, 1.f},
        {0.855f, 0.439f, 0.839f, 1.f},
        {0.094f, 0.624f, 1.f, 1.f}
    }};
    const auto non_code = find_lua_non_code(codepoints);
    struct open_delimiter {
        size_t position;
        char32_t closing;
    };
    std::vector<open_delimiter> open_delimiters;
    for (size_t index = 0; index < codepoints.size(); ++index) {
        if (non_code[index])
            continue;

        char32_t closing = U'\0';
        if (codepoints[index] == U'{')
            closing = U'}';
        else if (codepoints[index] == U'[')
            closing = U']';
        else if (codepoints[index] == U'(')
            closing = U')';

        if (closing != U'\0') {
            const size_t depth = open_delimiters.size();
            m_highlight_colors[index] = brace_colors[depth % brace_colors.size()];
            open_delimiters.push_back({index, closing});
            continue;
        }

        if (codepoints[index] == U'}' || codepoints[index] == U']' ||
            codepoints[index] == U')') {
            if (open_delimiters.empty() ||
                open_delimiters.back().closing != codepoints[index]) {
                m_highlight_colors[index] = brace_colors[0];
                continue;
            }

            const size_t opening = open_delimiters.back().position;
            open_delimiters.pop_back();
            m_highlight_colors[index] = m_highlight_colors[opening];
            if (codepoints[index] == U'}') {
                m_brace_pairs[opening] = index;
                m_brace_pairs[index] = opening;
                m_scope_pairs.emplace_back(opening, index);
            }
        }
    }

    if (!m_editor_guides_enabled)
        return;

    struct lua_scope {
        size_t opening;
        bool repeat_until;
    };
    std::vector<lua_scope> scopes;
    std::vector<size_t> pending_loops;
    auto add_scope = [&](size_t opening, size_t closing) {
        m_scope_pairs.emplace_back(opening, closing);
    };
    for (size_t index = 0; index < codepoints.size();) {
        if (non_code[index] ||
            !((codepoints[index] >= U'a' && codepoints[index] <= U'z') ||
              (codepoints[index] >= U'A' && codepoints[index] <= U'Z') ||
              codepoints[index] == U'_')) {
            ++index;
            continue;
        }

        const size_t token_start = index;
        while (index < codepoints.size() && !non_code[index] &&
               ((codepoints[index] >= U'a' && codepoints[index] <= U'z') ||
                (codepoints[index] >= U'A' && codepoints[index] <= U'Z') ||
                (codepoints[index] >= U'0' && codepoints[index] <= U'9') ||
                codepoints[index] == U'_'))
            ++index;

        const std::u32string token = codepoints.substr(token_start, index - token_start);
        if (token == U"function" || token == U"if") {
            scopes.push_back({token_start, false});
        } else if (token == U"repeat") {
            scopes.push_back({token_start, true});
        } else if (token == U"for" || token == U"while") {
            pending_loops.push_back(token_start);
        } else if (token == U"do") {
            const size_t opening = pending_loops.empty()
                ? token_start
                : pending_loops.back();
            if (!pending_loops.empty())
                pending_loops.pop_back();
            scopes.push_back({opening, false});
        } else if (token == U"end") {
            if (!scopes.empty() && !scopes.back().repeat_until) {
                add_scope(scopes.back().opening, token_start);
                scopes.pop_back();
            }
        } else if (token == U"until" && !scopes.empty() &&
                   scopes.back().repeat_until) {
            add_scope(scopes.back().opening, token_start);
            scopes.pop_back();
        }
    }
}

void bgui::text::on_update() {
    element::on_update();
    // If the style changes, update the font.
    if(m_last_font != computed_style.visual.font) {
        set_font(computed_style.visual.font);
        m_last_font = computed_style.visual.font;
    }
}

void bgui::text::calc_content_size(const layer&) {
    auto& font_manager = bgui::font_manager::get_instance();
    if (!font_manager.has_font(computed_style.visual.font)) {
        set_content_size({0, 0});
        return;
    }
    const auto& font = font_manager.get_font(computed_style.visual.font);
    if (font.chs.empty()) {
        set_content_size({0, 0});
        return;
    }

    const float scale = m_scale * bgui::get_global_scale();
    const float line_height = (font.ascent + font.descent + font.line_gap) * scale;
    const float wrap_width = 500.f * bgui::get_global_scale();
    float line_width = 0.f;
    float max_width = 0.f;
    int line_count = 1;

    for (char32_t character : utf8_to_utf32(m_buffer)) {
        if (character == U'\n') {
            max_width = std::max(max_width, line_width);
            line_width = 0.f;
            ++line_count;
            continue;
        }

        const float advance = glyph_advance(character, font, scale);
        if (m_wrap_enabled && line_width + advance > wrap_width && line_width > 0.f) {
            max_width = std::max(max_width, line_width);
            line_width = 0.f;
            ++line_count;
        }
        line_width += advance;
    }

    max_width = std::max(max_width, line_width);
    set_content_size({
        static_cast<int>(max_width + get_line_number_gutter_width(
            font, m_scale * bgui::get_global_scale())),
        static_cast<int>(line_count * line_height)
    });
}
void bgui::text::set_font(const std::string &path) {
    font_manager::get_instance().m_font_queue.push(path);
}

float bgui::text::get_text_width(const std::string& t) const {
    auto& font_manager = bgui::font_manager::get_instance();
    if (!font_manager.has_font(computed_style.visual.font))
        return 0.0f;
    const auto& font = font_manager.get_font(computed_style.visual.font);
    const auto& chs = font.chs;
    if (chs.empty()) return 0.0f;
    const float scale = m_scale * bgui::get_global_scale();

    float line_x = 0.f;
    float max_line_width = 0.0f;

    for (char32_t ca : utf8_to_utf32(t)) {
        if (ca == U'\n') {
            max_line_width = std::max(max_line_width, line_x);
            line_x = 0.f;
            continue;
        }

        line_x += glyph_advance(ca, font, scale);
    }

    // Garante que a última linha (que não termina em '\n') também seja considerada
    max_line_width = std::max(max_line_width, line_x);

    return max_line_width;
}

void bgui::text::get_requires(bgui::draw_data* data) {
    auto& font_manager = bgui::font_manager::get_instance();
    if (!font_manager.has_font(computed_style.visual.font))
        return;
    const auto& font = font_manager.get_font(computed_style.visual.font);
    const auto& chs = font.chs;
    if (chs.empty()) return;

    const float scale = m_scale * bgui::get_global_scale();
    float ascent   = font.ascent * scale;
    float descent  = font.descent * scale;
    float line_gap = font.line_gap * scale;

    float line_y = ascent;
    float line_x = 0.f;
    float max_line_width = 0.0f;
    int line_count = 1;
    int total_width = get_text_width(m_buffer);
    const float gutter_width = get_line_number_gutter_width(font, scale);
    const int combined_width = total_width + static_cast<int>(gutter_width);
    float text_origin_x = static_cast<float>(processed_x());
    switch (computed_style.layout.align.x) {
        case bgui::alignment::start:
            text_origin_x += gutter_width;
            break;
        case bgui::alignment::center:
            text_origin_x += (processed_width() - combined_width) / 2 + gutter_width;
            break;
        case bgui::alignment::end:
            text_origin_x += processed_width() - combined_width + gutter_width;
            break;
    }

    m_material.m_texture = font.atlas;

    // Converte a string inteira UMA única vez, evitando reconverter
    // e re-medir substrings dentro do loop (era O(n^2)) e evitando
    // usar índices de codepoint em std::string::substr (bug de bytes).
    std::vector<size_t> byte_lengths;
    std::u32string codepoints = utf8_to_utf32(m_buffer, &byte_lengths);
    size_t byte_position = 0;
    float cursor_x = 0.f;
    float cursor_y = 0.f;
    bool cursor_position_found = false;
    std::vector<std::optional<bgui::vec2>> codepoint_positions(codepoints.size());
    std::vector<float> line_indent_widths(codepoints.size(), 0.f);
    float current_indent_width = 0.f;
    bool at_line_indent = true;
    const auto space_glyph = chs.find(U' ');
    const float space_advance = space_glyph == chs.end()
        ? 0.f : space_glyph->second.advance * scale;

    auto draw_glyph = [&](
        char32_t character,
        float x,
        float y,
        std::optional<bgui::vec4> color_override
    ) {
        auto it = chs.find(character);
        if (it == chs.end()) return;
        const auto& ch = it->second;

        float xpos = text_origin_x + x + scale * ch.bearing[0];
        float ypos = processed_y() + y - (ch.bearing[1] * scale - ch.size[1] * scale);
        set_properties();
        data->enqueue({
            m_material, 6,
            { xpos, ypos, scale * ch.size[0], -scale * ch.size[1] },
            ch.uv_min, ch.uv_max,
        }, color_override);
    };

    auto draw_selection = [&](float x, float y, float width) {
        const float clip_left = std::max(
            static_cast<float>(processed_x()), static_cast<float>(data->m_clip_rect.x));
        const float clip_top = std::max(
            static_cast<float>(processed_y()), static_cast<float>(data->m_clip_rect.y));
        const float clip_right = std::min(
            static_cast<float>(processed_x() + processed_width()),
            static_cast<float>(data->m_clip_rect.x + data->m_clip_rect.z));
        const float clip_bottom = std::min(
            static_cast<float>(processed_y() + processed_height()),
            static_cast<float>(data->m_clip_rect.y + data->m_clip_rect.w));
        const float left = std::max(x, clip_left);
        const float top = std::max(y, clip_top);
        const float right = std::min(x + std::max(width, 2.f), clip_right);
        const float bottom = std::min(y + ascent + descent + line_gap, clip_bottom);
        if (right <= left || bottom <= top)
            return;

        const auto previous_color = data->m_draw_list.get_color();
        data->m_draw_list.set_color({0.2f, 0.45f, 0.8f, 0.35f});
        data->m_draw_list.add_rect_filled({left, top}, {right, bottom});
        data->m_draw_list.set_color(previous_color);
    };

    auto draw_line_number = [&](int number) {
        if (!m_editor_guides_enabled)
            return;
        const std::string label = std::to_string(number);
        float label_width = 0.f;
        for (char digit : label) {
            const auto glyph = chs.find(static_cast<unsigned char>(digit));
            if (glyph != chs.end())
                label_width += glyph->second.advance * scale;
        }
        float number_x = -5.f * bgui::get_global_scale() - label_width;
        const bgui::vec4 number_color{0.53f, 0.53f, 0.53f, 1.f};
        for (char digit : label) {
            const char32_t character = static_cast<unsigned char>(digit);
            draw_glyph(character, number_x, line_y, number_color);
            if (const auto glyph = chs.find(character); glyph != chs.end())
                number_x += glyph->second.advance * scale;
        }
    };
    draw_line_number(line_count);

    for (size_t index = 0; index < codepoints.size(); ++index) {
        const char32_t ca = codepoints[index];
        // Mede o glifo atual para decidir, incrementalmente, se ele
        // ainda cabe na linha (equivalente ao antigo "> 100", mas
        // sem refazer a conversão/medida do zero a cada caractere).
        const float advance_this_char = ca == U'\n'
            ? 0.f : glyph_advance(ca, font, scale);

        if (ca == U'\n') {
            if (byte_position >= m_selection_start && byte_position < m_selection_end) {
                draw_selection(
                    text_origin_x + line_x, processed_y() + line_y - ascent, space_advance);
            }
            if (m_cursor_visible && byte_position == m_cursor_position) {
                cursor_x = line_x;
                cursor_y = line_y;
                cursor_position_found = true;
            }
            // End of line
            max_line_width = std::max(max_line_width, line_x);
            line_y += (ascent + descent + line_gap);
            line_x = 0.f;
            current_indent_width = 0.f;
            at_line_indent = true;
            line_count++;
            byte_position += byte_lengths[index];
            draw_line_number(line_count);
            continue;
        }

        if (m_wrap_enabled && line_x + advance_this_char > 500.f) {
            max_line_width = std::max(max_line_width, line_x);
            line_y += (ascent + descent + line_gap);
            line_x = 0.f;
            line_count++;
        }
        if (byte_position >= m_selection_start && byte_position < m_selection_end) {
            draw_selection(
                text_origin_x + line_x, processed_y() + line_y - ascent, advance_this_char);
        }
        if (m_cursor_visible && byte_position == m_cursor_position) {
            cursor_x = line_x;
            cursor_y = line_y;
            cursor_position_found = true;
        }

        const auto color_override =
            index < m_highlight_colors.size() ? m_highlight_colors[index] : std::nullopt;
        line_indent_widths[index] = current_indent_width;
        codepoint_positions[index] = bgui::vec2{text_origin_x + line_x, line_y};
        if (at_line_indent && (ca == U' ' || ca == U'\t'))
            current_indent_width += ca == U'\t' ? space_advance * 4.f : advance_this_char;
        else
            at_line_indent = false;
        draw_glyph(ca, line_x, line_y, color_override);
        line_x += advance_this_char;
        byte_position += byte_lengths[index];
    }

    if (m_cursor_visible && byte_position == m_cursor_position) {
        cursor_x = line_x;
        cursor_y = line_y;
        cursor_position_found = true;
    }

    if (cursor_position_found) {
        const float caret_offset = scale * font.chs.at(U'|').bearing[0];
        draw_glyph(U'|', cursor_x - caret_offset, cursor_y, std::nullopt);
    }

    const float line_height = ascent + descent + line_gap;
    const float guide_left = static_cast<float>(processed_x());
    const float guide_right = static_cast<float>(processed_x() + processed_width());
    const float guide_top = static_cast<float>(processed_y());
    const float guide_bottom = static_cast<float>(processed_y() + processed_height());
    if (m_editor_guides_enabled) {
        const auto previous_draw_color = data->m_draw_list.get_color();
        for (const auto& [opening, closing] : m_scope_pairs) {
            if (opening >= codepoint_positions.size() || closing >= codepoint_positions.size() ||
                !codepoint_positions[opening] || !codepoint_positions[closing])
                continue;

            const auto& opening_position = *codepoint_positions[opening];
            const auto& closing_position = *codepoint_positions[closing];
            const bool brace_scope = m_brace_pairs[opening] &&
                                     *m_brace_pairs[opening] == closing;
            const float guide_x = brace_scope
                ? opening_position.x - 3.f * bgui::get_global_scale()
                : text_origin_x + line_indent_widths[opening] + space_advance * 4.f;
            if (guide_x < guide_left || guide_x >= guide_right)
                continue;

            const bgui::vec4 guide_color = brace_scope && m_highlight_colors[opening]
                ? bgui::vec4{
                    m_highlight_colors[opening]->r,
                    m_highlight_colors[opening]->g,
                    m_highlight_colors[opening]->b,
                    0.65f}
                : bgui::vec4{0.45f, 0.55f, 0.68f, 0.55f};
            const float opening_y = std::clamp(
                static_cast<float>(processed_y()) + opening_position.y - ascent +
                    line_height * 0.5f,
                guide_top, guide_bottom);
            const float closing_y = std::clamp(
                static_cast<float>(processed_y()) + closing_position.y - ascent +
                    line_height * 0.5f,
                guide_top, guide_bottom);

            data->m_draw_list.set_color(guide_color);
            if (closing_y > opening_y)
                data->m_draw_list.add_line({guide_x, opening_y}, {guide_x, closing_y}, 1.f);
            if (brace_scope) {
                const float marker_width = 4.f * bgui::get_global_scale();
                data->m_draw_list.add_line(
                    {std::max(guide_left, guide_x - marker_width), opening_y},
                    {std::min(guide_right, guide_x + marker_width), opening_y}, 1.f);
                data->m_draw_list.add_line(
                    {std::max(guide_left, guide_x - marker_width), closing_y},
                    {std::min(guide_right, guide_x + marker_width), closing_y}, 1.f);
            }
        }
        data->m_draw_list.set_color(previous_draw_color);
    }

    max_line_width = std::max(max_line_width, line_x);
    int total_height = line_count * (ascent + descent + line_gap);

    set_content_size({combined_width, total_height});
}