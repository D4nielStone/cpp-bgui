#include "os/font.hpp"
#include <stdexcept>
#include <filesystem>
#include <iostream>

bos::font_manager::font_manager() {
    if (FT_Init_FreeType(&ft)) {
        throw std::runtime_error("Error initializing Freetype.");
    }
}

bos::font_manager::~font_manager() {
    m_fonts.clear();
    FT_Done_FreeType(ft);
}

bos::font &bos::font_manager::load_font(const std::string &font_name, const std::string &font_path, FT_UInt resolution) {
    if (has_font(font_name) && m_fonts[font_name].path == font_path)
        return m_fonts[font_name];

    FT_Face face{};
    if (FT_New_Face(ft, font_path.c_str(), 0, &face))
        throw std::runtime_error("Error loading font: " + font_name);
    
        FT_Set_Pixel_Sizes(face, 0u, resolution);
    
    bos::font _font{};
    _font.path = font_path;
    
    int padding = 2;
    int width = 0;
    int max_height = 0;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    // First pass: compute total atlas size
    for (unsigned int c = 32; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;
    
        width += face->glyph->bitmap.width + padding;
        max_height = std::max(max_height, (int)face->glyph->bitmap.rows);
    }
    
    // Create atlas texture
    unsigned int atlasTex = 0;
    glGenTextures(1, &atlasTex);
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, max_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Fill atlas
    int xOffset = 0;
    for (unsigned int c = 32; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;
    
        FT_Bitmap& bmp = face->glyph->bitmap;
    
        int yOffset = max_height - face->glyph->bitmap_top;
        glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, bmp.width, bmp.rows, GL_RED, GL_UNSIGNED_BYTE, bmp.buffer);

    
        // Compute normalized UVs
        float u0 = xOffset / (float)width;      // left
        float u1 = (xOffset + bmp.width) / (float)width;    // right
        float v0 = (float)yOffset / (float)max_height;                  // topo normalizado
        float v1 = (float)(yOffset + bmp.rows) / (float)max_height;     // fundo normalizado

    
        // Store glyph
        bos::character ch{};
        ch.m_tex = atlasTex;
        ch.size = { bmp.width, bmp.rows };
        ch.bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
        ch.advance = face->glyph->advance.x >> 6;
        ch.uv_min = { u0, v0 };
        ch.uv_max = { u1, v1 };
    
        _font.chs.emplace(c, ch);
        xOffset += bmp.width + padding;
    }
    
    _font.atlas = atlasTex;
    _font.atlas_size = { (float)width, (float)max_height };
    m_fonts.emplace(font_name, _font);
    FT_Done_Face(face);
    return m_fonts[font_name];
}


bool bos::font_manager::has_font(const std::string &name) {
    return m_fonts.find(name) != m_fonts.end();
}
bos::font &bos::font_manager::get_font(const std::string& name) {
    if (has_font(name)) return m_fonts[name];
    else throw std::runtime_error("trying to get an invalid font.");
}

int bos::get_text_size(const std::string &font_name, const std::string &phrase) {
    auto _font = font_manager::instance().get_font(font_name);
  
     int max_width = 0;
     int line_width = 0;
  
     for (char32_t c : phrase) {
         if (c == '\n') {
             // end the line
             max_width = std::max(max_width, line_width);
             line_width = 0;
             continue;
         }
        if (_font.chs.find(c) != _font.chs.end()) {
            const character& ch = _font.chs[c];
            line_width += (ch.advance);
        }
    }

    // compare the last line
    max_width = std::max(max_width, line_width);
    return max_width;
}