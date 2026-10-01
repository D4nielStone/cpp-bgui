#pragma once

#include "elem/element.hpp"
#include <functional>

namespace bgui {
    /**
     * @brief A UI element that renders a texture-backed image.
     */
    class image final : public element {
    public:
        image();

        /**
         * @brief Creates an image from an already prepared texture.
         * @param texture Texture data or an external GPU texture.
         * @param on_click Optional callback invoked when the image is clicked.
         */
        explicit image(const texture& texture, std::function<void(const std::string&)> on_click = nullptr);

        /**
         * @brief Creates an image by loading a file relative to the assets directory.
         * @param texture_path Asset filename or path relative to the assets directory.
         * @param on_click Optional callback invoked when the image is clicked.
         */
        explicit image(const std::string& texture_path, std::function<void(const std::string&)> on_click = nullptr);

        /**
         * @brief Replaces the image texture and marks the element for style updates.
         */
        void set_texture(const texture& texture);

        /** Sets the requested image size in logical pixels. */
        void set_size(float width, float height);

        /** Sets the requested size mode for both image axes. */
        void set_size_mode(mode width_mode, mode height_mode);

        /** Uses the decoded texture dimensions as the requested image size. */
        void use_natural_size();

        /**
         * @brief Uses a texture owned and created by the caller's graphics backend.
         */
        void set_external_texture(
            unsigned int texture_id,
            const vec2& size,
            bool flip_vertical = true
        );

        const texture& get_texture() const { return m_texture; }
        void calc_content_size(const layer& lay) override;
        void get_requires(draw_data* calls) override;
        void on_clicked() override {
            on_clicked();
            if (m_on_click) {
                m_on_click(m_texture.m_path);
            }
        }
    private:
        std::function<void(const std::string&)> m_on_click;
        texture m_texture{};
        bool m_flip_vertical{true};
    };
}
