#pragma once
#include <glad/glad.h>
#include <string>
#include <utility>
#include <unordered_map>
#include "utils/vec.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace bos {
    struct character {
        GLuint m_tex{0};
        butil::vec<2, unsigned int> size{0, 0};
        butil::vec<2, FT_Int> bearing{0, 0};
        long int advance{0};
        butil::vec2 uv_min{0.f};
        butil::vec2 uv_max{0.f};
    };

    struct font {
        std::string path{""};        
        GLuint atlas{0};
        std::unordered_map<char32_t, bos::character> chs{};
        butil::vec2 atlas_size{0.f, 0.f};
    };

    class font_manager {
    private:
        std::unordered_map<std::string, font> m_fonts;
        FT_Library ft;
    public:
        static font_manager& instance() {
            static font_manager f;
            return f;
        }
        font_manager();
        ~font_manager();
        font& load_font(const std::string& font_name, const std::string& font_path, FT_UInt resolution);
        font& get_font(const std::string& name);
        bool has_font(const std::string& name);
    };
    static int get_text_size(const std::string& font_name, const std::string& phrase);
} // namespace bos