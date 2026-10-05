#pragma once

#include "elem/element.hpp"

namespace bgui {
    class progress_bar : public element {
    public:
        explicit progress_bar(
            float minimum = 0.f,
            float maximum = 100.f,
            float value = 0.f,
            orientation direction = orientation::horizontal
        );

        float get_minimum() const noexcept { return m_minimum; }
        float get_maximum() const noexcept { return m_maximum; }
        float get_value() const noexcept { return m_value; }
        orientation get_orientation() const noexcept { return m_orientation; }

        void set_range(float minimum, float maximum);
        void set_value(float value);
        void get_requires(draw_data* calls) override;

    private:
        float m_minimum;
        float m_maximum;
        float m_value;
        orientation m_orientation;
        material m_fill_material;
    };
}
