#pragma once
#include "element.hpp"
#include "utils/draw.hpp"
#include "utils/style.hpp"
#include "os/font.hpp"

namespace bgui {
    class text : public element {
    private:
        std::string m_buffer, m_last_font;
        float m_scale;
    public:
        text(const std::string& buffer, float scale);
        ~text();
        // sets to this text the font with `name`.
        void set_font(const std::string& name);
        void on_update() override;
        float get_text_width();
        void set_buffer(const std::string& buffer) { m_buffer = buffer; };
        const std::string& get_buffer() const { return m_buffer; };
        void get_requires(bgui::draw_data *calls) override;
    };
} // namespace bgui