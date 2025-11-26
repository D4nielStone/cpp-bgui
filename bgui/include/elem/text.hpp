#pragma once
#include "element.hpp"
#include "utils/theme.hpp"
#include "os/font.hpp"

namespace elements {
    class text : public element {
    private:
        std::string m_buffer;
        std::string m_font_name;
        float m_scale;
        bos::font m_font;
    public:
        text(const std::string& buffer, float scale);
        ~text();
        // sets to this text the font with `name`.
        void set_font(const std::string& name);
        void update() override;
        void get_draw_requests(std::vector<butil::draw_request>& calls) override;
        void apply_theme(const butil::theme& t) override {
            m_material.set("u_text_color", t.m_text_color);
            m_material.m_visible = true;
        };
    };
} // namespace elements