#include "utils/syntax_highlight.hpp"

#include "os/asset_manager.hpp"

#include <charconv>
#include <cctype>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

namespace {
    class json_parser {
    public:
        explicit json_parser(std::string source) : m_source(std::move(source)) {}

        std::vector<bgui::syntax_highlight_rule> parse() {
            expect('{');
            bool found_rules = false;
            std::vector<bgui::syntax_highlight_rule> rules;

            skip_whitespace();
            while (!consume('}')) {
                const std::string key = parse_string();
                expect(':');
                if (key != "rules")
                    fail("Unknown root property '" + key + "'");
                if (found_rules)
                    fail("Duplicate 'rules' property");
                rules = parse_rules();
                found_rules = true;
                skip_whitespace();
                if (consume('}'))
                    break;
                expect(',');
                skip_whitespace();
            }

            skip_whitespace();
            if (m_position != m_source.size())
                fail("Unexpected content after the root object");
            if (!found_rules)
                fail("Missing required 'rules' array");
            return rules;
        }

    private:
        std::string m_source;
        size_t m_position = 0;

        [[noreturn]] void fail(const std::string& message) const {
            throw std::runtime_error(
                "[SyntaxHighlight] Invalid JSON at byte " +
                std::to_string(m_position) + ": " + message);
        }

        void skip_whitespace() {
            while (m_position < m_source.size() &&
                   std::isspace(static_cast<unsigned char>(m_source[m_position])))
                ++m_position;
        }

        bool consume(char expected) {
            skip_whitespace();
            if (m_position < m_source.size() && m_source[m_position] == expected) {
                ++m_position;
                return true;
            }
            return false;
        }

        void expect(char expected) {
            if (!consume(expected))
                fail(std::string("Expected '") + expected + "'");
        }

        static void append_utf8(std::string& output, unsigned int codepoint) {
            if (codepoint <= 0x7F) {
                output.push_back(static_cast<char>(codepoint));
            } else if (codepoint <= 0x7FF) {
                output.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
                output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
            } else if (codepoint <= 0xFFFF &&
                       !(codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
                output.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
                output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
                output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
            } else if (codepoint <= 0x10FFFF) {
                output.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
                output.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
                output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
                output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
            } else {
                throw std::runtime_error("[SyntaxHighlight] Invalid Unicode escape in JSON string");
            }
        }

        unsigned int parse_hex_quad() {
            if (m_source.size() - m_position < 4)
                fail("Incomplete Unicode escape");
            unsigned int value = 0;
            const char* begin = m_source.data() + m_position;
            const auto result = std::from_chars(begin, begin + 4, value, 16);
            if (result.ec != std::errc{} || result.ptr != begin + 4)
                fail("Invalid Unicode escape");
            m_position += 4;
            return value;
        }

        std::string parse_string() {
            expect('"');
            std::string value;
            while (m_position < m_source.size()) {
                const char current = m_source[m_position++];
                if (current == '"')
                    return value;
                if (static_cast<unsigned char>(current) < 0x20)
                    fail("Control character in JSON string");
                if (current != '\\') {
                    value.push_back(current);
                    continue;
                }
                if (m_position >= m_source.size())
                    fail("Incomplete escape sequence");
                switch (m_source[m_position++]) {
                    case '"': value.push_back('"'); break;
                    case '\\': value.push_back('\\'); break;
                    case '/': value.push_back('/'); break;
                    case 'b': value.push_back('\b'); break;
                    case 'f': value.push_back('\f'); break;
                    case 'n': value.push_back('\n'); break;
                    case 'r': value.push_back('\r'); break;
                    case 't': value.push_back('\t'); break;
                    case 'u': {
                        unsigned int codepoint = parse_hex_quad();
                        if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
                            if (m_source.size() - m_position < 6 ||
                                m_source[m_position] != '\\' ||
                                m_source[m_position + 1] != 'u')
                                fail("High surrogate without a low surrogate");
                            m_position += 2;
                            const unsigned int low_surrogate = parse_hex_quad();
                            if (low_surrogate < 0xDC00 || low_surrogate > 0xDFFF)
                                fail("Invalid low surrogate");
                            codepoint = 0x10000 +
                                ((codepoint - 0xD800) << 10) +
                                (low_surrogate - 0xDC00);
                        } else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF) {
                            fail("Low surrogate without a high surrogate");
                        }
                        append_utf8(value, codepoint);
                        break;
                    }
                    default: fail("Invalid string escape");
                }
            }
            fail("Unterminated JSON string");
        }

        bgui::vec4 parse_color(const std::string& value) {
            if ((value.size() != 7 && value.size() != 9) || value.front() != '#')
                fail("Colors must use #RRGGBB or #RRGGBBAA format");

            auto component = [&](size_t offset) {
                unsigned int channel = 0;
                const char* begin = value.data() + offset;
                const auto result = std::from_chars(begin, begin + 2, channel, 16);
                if (result.ec != std::errc{} || result.ptr != begin + 2)
                    fail("Invalid hexadecimal color");
                return static_cast<float>(channel) / 255.f;
            };
            return {
                component(1), component(3), component(5),
                value.size() == 9 ? component(7) : 1.f
            };
        }

        std::vector<bgui::syntax_highlight_rule> parse_rules() {
            expect('[');
            std::vector<bgui::syntax_highlight_rule> rules;
            if (consume(']'))
                return rules;

            do {
                rules.push_back(parse_rule());
            } while (consume(','));
            expect(']');
            return rules;
        }

        bgui::syntax_highlight_rule parse_rule() {
            expect('{');
            std::string pattern;
            std::string color_value;
            bool has_pattern = false;
            bool has_color = false;

            do {
                const std::string key = parse_string();
                expect(':');
                const std::string value = parse_string();
                if (key == "pattern") {
                    if (has_pattern)
                        fail("Duplicate 'pattern' property");
                    pattern = value;
                    has_pattern = true;
                } else if (key == "color") {
                    if (has_color)
                        fail("Duplicate 'color' property");
                    color_value = value;
                    has_color = true;
                } else {
                    fail("Unknown rule property '" + key + "'");
                }
            } while (consume(','));
            expect('}');

            if (!has_pattern || !has_color)
                fail("Every rule requires both 'pattern' and 'color'");
            try {
                return {pattern, std::regex(pattern), parse_color(color_value)};
            } catch (const std::regex_error& error) {
                throw std::runtime_error(
                    "[SyntaxHighlight] Invalid regex '" + pattern + "': " + error.what());
            }
        }
    };
}

std::vector<bgui::syntax_highlight_rule> bgui::load_syntax_highlight_config(
    const std::string& path
) {
    const auto& resolved_path = asset_manager::get_instance().resolve_asset_path(path);
    std::ifstream file(resolved_path, std::ios::binary);
    if (!file)
        throw std::runtime_error("[SyntaxHighlight] Could not open config: " + resolved_path);

    std::string source{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()};
    return json_parser(std::move(source)).parse();
}
