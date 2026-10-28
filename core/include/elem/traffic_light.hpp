#pragma once

#include "button.hpp"
#include <functional>

namespace bgui {

enum class traffic_light_type {
    close,
    minimize,
    maximize
};

class traffic_light final : public button {
public:
    traffic_light(
        traffic_light_type type,
        const std::function<void()>& callback
    )
        : button("", 0.3, callback),
          m_type(type) {
        receives_input(true);
        style.layout.require_size(12.f, 12.f);
        style.layout.require_mode(mode::pixel, mode::pixel);
    }

    void get_requires(draw_data* calls) override;

private:
    traffic_light_type m_type;
};

}