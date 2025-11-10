#pragma once
#include "element.hpp"
#include "utils/theme.hpp"
#include "os/font.hpp"

namespace elements {
    class text : public element {
    private:
        std::string m_buffer;
        std::string m_font_path;
        unsigned int m_scale;
        bos::font m_font;
    public:
        text(const std::string& buffer, unsigned int scale);
        ~text();
        // sets to this text the font with `path`.
        void set_font(const std::string& path);
        void update() override;
        void get_draw_calls(std::vector<draw_call>& calls) override;
        void set_theme(const butil::theme& t) override {
            m_material.set("u_text_color", t.m_text_color);
            m_material.m_visible = true;
        };
    };
} // namespace elements