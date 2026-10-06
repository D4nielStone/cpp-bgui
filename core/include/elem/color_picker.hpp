#pragma once

#include "elem/element.hpp"
#include <functional>

namespace bgui {
    class color_picker : public element {
    public:
        explicit color_picker(const color& initial_color = color{1.f, 0.f, 0.f, 1.f});

        const color& get_color() const noexcept { return m_color; }
        float get_hue() const noexcept { return m_hue; }

        void set_color(const color& value);
        void set_hue(float hue);
        void set_on_change(const std::function<void(const color&)>& callback);

        void on_pressed() override;
        void on_released() override;
        void on_mouse_hover() override;
        void set_drag(const vec2i& mouse_delta) override;
        void get_requires(draw_data* calls) override;

    private:
        enum class drag_target {
            none,
            hue,
            triangle
        };

        void update_from_pointer();
        void update_color();
        void update_palette();

        color m_color{1.f, 0.f, 0.f, 1.f};
        float m_hue{0.f};
        float m_white_weight{0.f};
        float m_hue_weight{1.f};
        float m_black_weight{0.f};
        drag_target m_drag_target{drag_target::none};
        texture m_marker;
        material m_marker_material;
        std::function<void(const color&)> m_on_change;
        bool m_palette_dirty{true};
    };
}
