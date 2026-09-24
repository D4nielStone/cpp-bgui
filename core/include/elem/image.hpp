#pragma once

#include "elem/element.hpp"

namespace bgui {
    class image final : public element {
    public:
        image();
        explicit image(const texture& texture);

        void set_texture(const texture& texture);
        void set_external_texture(
            unsigned int texture_id,
            const vec2& size,
            bool flip_vertical = true
        );

        const texture& get_texture() const { return m_texture; }
        void get_requires(draw_data* calls) override;

    private:
        texture m_texture{};
        bool m_flip_vertical{true};
    };
}
