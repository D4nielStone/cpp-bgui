#include "os/asset_manager.hpp"

#include <array>
#include <filesystem>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#else
#include <png.h>
#endif

namespace {
    std::filesystem::path find_asset_path(const std::string& path) {
        const std::filesystem::path input(path);
        if (input.is_absolute() && std::filesystem::is_regular_file(input))
            return input;

        const std::array<std::filesystem::path, 4> candidates = {
            input,
            std::filesystem::path("assets") / input,
            std::filesystem::path("../assets") / input,
#ifdef BGUI_ASSETS_DIR
            std::filesystem::path(BGUI_ASSETS_DIR) / input
#else
            std::filesystem::path()
#endif
        };

        for (const auto& candidate : candidates) {
            if (!candidate.empty() && std::filesystem::is_regular_file(candidate))
                return candidate;
        }

        throw std::runtime_error("[AssetManager] Could not find asset: " + path);
    }

#ifdef _WIN32
    using Microsoft::WRL::ComPtr;

    bgui::texture decode_texture(const std::filesystem::path& asset_path) {
        const HRESULT init_result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        struct com_apartment {
            bool initialized;
            ~com_apartment() {
                if (initialized) CoUninitialize();
            }
        } apartment{SUCCEEDED(init_result)};

        ComPtr<IWICImagingFactory> factory;
        HRESULT result = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&factory)
        );
        if (FAILED(result)) {
            throw std::runtime_error("[AssetManager] Could not initialize WIC");
        }

        ComPtr<IWICBitmapDecoder> decoder;
        result = factory->CreateDecoderFromFilename(
            asset_path.wstring().c_str(), nullptr, GENERIC_READ,
            WICDecodeMetadataCacheOnLoad, &decoder
        );

        ComPtr<IWICBitmapFrameDecode> frame;
        ComPtr<IWICFormatConverter> converter;
        if (SUCCEEDED(result)) result = decoder->GetFrame(0, &frame);
        if (SUCCEEDED(result)) result = factory->CreateFormatConverter(&converter);
        if (SUCCEEDED(result)) {
            result = converter->Initialize(
                frame.Get(), GUID_WICPixelFormat32bppRGBA,
                WICBitmapDitherTypeNone, nullptr, 0.0,
                WICBitmapPaletteTypeCustom
            );
        }

        UINT width = 0;
        UINT height = 0;
        if (SUCCEEDED(result)) result = converter->GetSize(&width, &height);

        bgui::texture loaded;
        loaded.m_path = asset_path.string();
        loaded.m_has_alpha = true;
        loaded.m_size = {static_cast<float>(width), static_cast<float>(height)};
        loaded.m_buffer.resize(static_cast<size_t>(width) * height * 4);
        if (SUCCEEDED(result)) {
            result = converter->CopyPixels(
                nullptr, width * 4,
                static_cast<UINT>(loaded.m_buffer.size()),
                loaded.m_buffer.data()
            );
        }

        if (FAILED(result))
            throw std::runtime_error("[AssetManager] Could not decode asset: " + asset_path.string());
        return loaded;
    }
#else
    bgui::texture decode_texture(const std::filesystem::path& asset_path) {
        png_image image{};
        image.version = PNG_IMAGE_VERSION;
        if (!png_image_begin_read_from_file(&image, asset_path.string().c_str()))
            throw std::runtime_error("[AssetManager] Could not decode asset: " + asset_path.string());

        image.format = PNG_FORMAT_RGBA;
        bgui::texture loaded;
        loaded.m_path = asset_path.string();
        loaded.m_has_alpha = true;
        loaded.m_size = {
            static_cast<float>(image.width),
            static_cast<float>(image.height)
        };
        loaded.m_buffer.resize(PNG_IMAGE_SIZE(image));

        if (!png_image_finish_read(&image, nullptr, loaded.m_buffer.data(), 0, nullptr)) {
            const std::string message = image.message;
            png_image_free(&image);
            throw std::runtime_error("[AssetManager] Could not read asset: " + message);
        }
        png_image_free(&image);
        return loaded;
    }
#endif
}

namespace bgui {
    asset_manager& asset_manager::get_instance() {
        static asset_manager instance;
        return instance;
    }

    std::string asset_manager::make_font_key(
        const std::string& name,
        const unsigned int resolution
    ) {
        return name + "#" + std::to_string(resolution);
    }

    const std::string& asset_manager::resolve_asset_path(const std::string& path) const {
        const auto cached = m_resolved_paths.find(path);
        if (cached != m_resolved_paths.end())
            return cached->second;

        const auto resolved = find_asset_path(path).string();
        return m_resolved_paths.emplace(path, resolved).first->second;
    }

    texture& asset_manager::load_texture(const std::string& path) {
        const auto& resolved_path = resolve_asset_path(path);
        const auto cached = m_textures.find(resolved_path);
        if (cached != m_textures.end())
            return cached->second;

        auto [it, inserted] = m_textures.emplace(resolved_path, decode_texture(resolved_path));
        return it->second;
    }

    bool asset_manager::has_texture(const std::string& path) const {
        const auto resolved_path = resolve_asset_path(path);
        return m_textures.find(resolved_path) != m_textures.end();
    }

    font& asset_manager::store_font(
        const std::string& name,
        const unsigned int resolution,
        font value
    ) {
        if (resolution == 0)
            throw std::invalid_argument("Font resolution must be greater than zero.");
        auto [it, inserted] = m_fonts.emplace(make_font_key(name, resolution), std::move(value));
        return it->second;
    }

    bool asset_manager::has_font(
        const std::string& name,
        const unsigned int resolution
    ) const {
        if (name == "default") {
            const auto default_it = m_default_font_keys.find(resolution);
            return default_it != m_default_font_keys.end() &&
                m_fonts.find(default_it->second) != m_fonts.end();
        }
        return m_fonts.find(make_font_key(name, resolution)) != m_fonts.end();
    }

    font& asset_manager::get_font(
        const std::string& name,
        const unsigned int resolution
    ) {
        if (resolution == 0)
            throw std::invalid_argument("Font resolution must be greater than zero.");

        if (name == "default") {
            const auto default_it = m_default_font_keys.find(resolution);
            if (default_it != m_default_font_keys.end()) {
                const auto font_it = m_fonts.find(default_it->second);
                if (font_it != m_fonts.end())
                    return font_it->second;
            }
        }

        const auto it = m_fonts.find(make_font_key(name, resolution));
        if (it == m_fonts.end())
            throw std::runtime_error("Font " + name + " at resolution " +
                                     std::to_string(resolution) + " was not found.");
        return it->second;
    }

    void asset_manager::set_default_font(
        const std::string& name,
        const unsigned int resolution
    ) {
        const auto key = make_font_key(name, resolution);
        if (m_fonts.find(key) == m_fonts.end())
            throw std::runtime_error("Cannot set an unloaded font as the default.");
        m_default_font_keys[resolution] = key;
    }
}
