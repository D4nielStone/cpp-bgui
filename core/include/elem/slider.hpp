#pragma once

#include "elem/element.hpp"
#include <functional>

namespace bgui {
    class slider : public element {
    public:
        explicit slider(
            float minimum = 0.f,
            float maximum = 1.f,
            float value = 0.f,
            orientation direction = orientation::horizontal
        );

        float get_minimum() const noexcept { return m_minimum; }
        float get_maximum() const noexcept { return m_maximum; }
        float get_value() const noexcept { return m_value; }
        float get_step() const noexcept { return m_step; }
        orientation get_orientation() const noexcept { return m_orientation; }

        void set_range(float minimum, float maximum);
        void set_value(float value);
        void set_step(float step);
        void set_on_change(const std::function<void(float)>& callback);

        void on_pressed() override;
        void on_clicked() override;
        void on_released() override;
        void on_mouse_hover() override;
        void set_drag(const vec2i& mouse_delta) override;
        void get_requires(draw_data* calls) override;

    private:
        float normalize_value(float value) const;
        void update_from_pointer();

        float m_minimum;
        float m_maximum;
        float m_value;
        float m_step{0.f};
        orientation m_orientation;
        std::function<void(float)> m_on_change;
        material m_fill_material;
        material m_thumb_material;
    };
}
