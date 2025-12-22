#pragma once
#include "element.hpp"
#include "utils/draw.hpp"
#include "utils/style.hpp"
#include "os/font.hpp"

namespace bgui {
    class text : public element {
    private:
        std::string m_buffer;
        std::string m_font_name;
        alignment m_self_alignment;
        float m_scale;
        bgui::font m_font;
    public:
        text(const std::string& buffer, float scale);
        ~text();
        void set_alignment(const alignment& align) { m_self_alignment = align; };
        // sets to this text the font with `name`.
        void set_font(const std::string& name);
        void update() override;
        float get_text_width();
        void set_buffer(const std::string& buffer) { m_buffer = buffer; };
        const std::string& get_buffer() const { return m_buffer; };
        void get_requests(bgui::draw_data *calls) override;
        void apply_style(const bgui::style& t) override {
            element::apply_style(t);
            // TODO: adds pointer to set material props
            m_material.set("text_color", m_style.m_text_color);
        };
    };
} // namespace bgui