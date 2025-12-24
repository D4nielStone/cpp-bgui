#pragma once
#include <vector>
#include <memory>
#include <limits.h>

#include "utils/style.hpp"
#include "utils/draw.hpp"

namespace bgui {
    union uid {
        inline static unsigned int s_nextid {0};
        unsigned int m_uint;
        const char* m_str;
        uid() : m_uint(s_nextid++) {}
        uid(const char* s)  : m_str(s+s_nextid++) {}
    };
    class layout;
    struct border {
        vec2i m_size = vec2i(1);
        color m_color = color(1.f);
        float m_radius = 2.f;
    };

    class element {
    protected:
        bool m_has_tag;
        bgui::uid m_uid;

        layout* m_parent {nullptr};
        material m_material;
        bool m_visible {true};
        bool m_recives_input{true};

        vec<2, mode> m_requested_mode {mode::pixel, mode::pixel};
        vec2  m_requested_size {0.f, 0.f};

        vec4i m_padding {0,0,0,0}; // left, top, right, bottom
        vec4i m_margin  {0,0,0,0}; // left, top, right, bottom
        border m_border;
        style m_style;

        vec2i m_min_size {10, 10};
        vec2i m_max_size {INT_MAX, INT_MAX};
        vec2i m_last_drag{0, 0};

        // FINAL COMPUTED RECT (layout writes this)
        vec4i m_rect {0, 0, 0, 0}; // x, y, width, height
    public:
        element();
        /// @brief Tag Consturctor
        /// Creates a uid based on const char* type.
        element(const char* tag);
        virtual ~element() = default;

        // Margin
        void set_margin(int left, int top, int right, int bottom);
        void set_margin(int horizontal, int vertical);

        // Padding
        void set_padding(int left, int top, int right, int bottom);
        void set_padding(int horizontal, int vertical);

        // Borders
        void set_border_size(int x, int y);
        void set_border_color(const color& c);
        void set_border_radius(float radius);

        // Size constraints
        void set_min_size(int width, int height);
        void set_max_size(int width, int height);

        // Requested size
        void request_size(float width, float height);
        void request_width(mode m, float value = 100.f);
        void request_height(mode m, float value = 100.f);

        // Material / visibility
        void set_shader_tag(const std::string& shd);
        void set_material(const material& m);
        void set_visible(bool v);

        void set_parent(layout* p) { m_parent = p; }

        vec2i get_min_size() const { return m_min_size; }
        vec2i get_max_size() const { return m_max_size; }

        // Final rect (computed by layout)
        int processed_x()     const { return m_rect[0]; }
        int processed_y()     const { return m_rect[1]; }
        int processed_width() const { return m_rect[2]; }
        int processed_height()const { return m_rect[3]; }

        void set_final_rect(int x, int y, int w, int h) {m_rect = vec4i({x, y, w, h}); }
        void set_position(int x, int y) {m_rect[0] = x; m_rect[1] = y;}
        void set_final_size(int x, int y) {m_rect[2] = x; m_rect[3] = y;}

        vec2i processed_position() const { return vec2i({m_rect[0], m_rect[1]}); }
        vec2i processed_size()     const { return vec2i({m_rect[2], m_rect[3]}); }
        vec4i processed_rect()     const { return m_rect; }

        // Requested size
        vec2 requested_size() const { return m_requested_size; }
        vec<2,mode> get_requested_mode() const { return m_requested_mode; }

        // Margin / Padding / Border
        vec4i get_margin()  const { return m_margin; }
        vec4i get_padding() const { return m_padding; }
        border get_border() const { return m_border; }

        // style / material
        std::string get_shader_tag() const;
        material& get_material() { return m_material; }
        style& get_style() {
            return m_style;
        }

        // hierarchy
        layout* get_parent() const { return m_parent; }
        virtual layout* as_layout() { return nullptr; }

        // input
        void recives_input(bool b) {
            m_recives_input = b;
        };
        bool recives_input() const {
            return m_recives_input;
        }
        vec2i get_drag() const;
        
        // id
        unsigned int get_id() const {
            if(!m_has_tag)
                return m_uid.m_uint;
            else return static_cast<unsigned int>(*m_uid.m_str);
        }

        void update_size(const vec2i& available_size);
        virtual void update();
        virtual void apply_style(const style& style){ 
            m_style = style;
            m_material.set("bg_color", m_style.m_box_color);
            m_material.set("bordered", true);
            m_material.set("border_radius", 4.f);
            m_material.set("border_size", 1.f);
            m_material.set("border_color", m_style.m_button_border_color);
         };
        virtual float content_width(){return 0.f;};
        virtual float content_height(){return 0.f;};
        virtual void on_pressed() {};
        virtual void on_drag(const vec2i& mouse_delta) {m_last_drag = mouse_delta;};
        virtual void on_clicked() {};
        virtual void on_released() {};
        virtual void on_mouse_hover() {};
        virtual vec2i get_content_size() {return get_min_size(); }
        virtual void get_requests(draw_data* calls);
    };

} // namespace bgui