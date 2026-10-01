#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "os/font.hpp"
#include "utils/texture.hpp"

namespace bgui {
    /**
     * @brief Central cache and loader for backend-neutral image and font assets.
     *
     * The manager stores CPU-side asset data only. Graphics backends remain
     * responsible for uploading textures and rendering font atlases.
     */
    class asset_manager {
    public:
        static asset_manager& get_instance();

        asset_manager(const asset_manager&) = delete;
        asset_manager& operator=(const asset_manager&) = delete;

        /** Load an image from the configured assets directories or return its cache entry. */
        texture& load_texture(const std::string& path);
        bool has_texture(const std::string& path) const;

        /** Store and retrieve backend-generated font data by name and resolution. */
        font& store_font(const std::string& name, unsigned int resolution, font value);
        font& get_font(const std::string& name, unsigned int resolution);
        bool has_font(const std::string& name, unsigned int resolution) const;

        void set_default_font(const std::string& name, unsigned int resolution);
        const std::string& resolve_asset_path(const std::string& path) const;

    private:
        asset_manager() = default;

        static std::string make_font_key(const std::string& name, unsigned int resolution);

        std::unordered_map<std::string, texture> m_textures;
        std::unordered_map<std::string, font> m_fonts;
        std::unordered_map<unsigned int, std::string> m_default_font_keys;
        mutable std::unordered_map<std::string, std::string> m_resolved_paths;
    };
}
