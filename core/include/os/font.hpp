#pragma once
//#include <GL/glew.h>
#include <string>
#include <utility>
#include <unordered_map>
#include "utils/vec.hpp"
/*#include <ft2build.h>
#include FT_FREETYPE_H*/

namespace bos {
    struct character {
        butil::vec<2, unsigned int> size{0, 0};
        butil::vec<2, int> bearing{0, 0};
        long int advance{0};
        butil::vec2 uv_min{0.f};
        butil::vec2 uv_max{0.f};
    };

    struct font {
        std::string path{""};        
        // GLuint atlas{0}; TODO: use bgui texture manager
        std::unordered_map<char32_t, bos::character> chs{};
        butil::vec2 atlas_size{0.f, 0.f};
        float ascent = 0.0f;
        float descent = 0.0f;
        float line_gap = 0.0f;
    };

    class font_manager {
    private:
        std::unordered_map<std::string, std::string> m_system_fonts;
        std::unordered_map<std::string, font> m_fonts;
        //FT_Library ft;
    public:
        static constexpr float m_default_resolution = 40.f;
        static font_manager& instance() {
            static font_manager f;
            return f;
        }
        font_manager();
        ~font_manager();
        void search_system_fonts();
        font& load_font(const std::string& font_name, const std::string& font_path, unsigned int resolution);
        font& get_font(const std::string& name);
        bool has_font(const std::string& name);
    };
    static int get_text_size(const std::string& font_name, const std::string& phrase);
} // namespace bos