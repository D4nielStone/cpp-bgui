#include <os/resource_files.hpp>
#include "os/asset_manager.hpp"

#include <array>
#include <filesystem>
#include <stdexcept>
#include <fstream>
#include <memory>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#else
#include <png.h>
#include <cstdio>
#include <csetjmp>
#include <jpeglib.h>
#endif

namespace {
    std::filesystem::path find_asset_path(const std::string& path) {
        const std::filesystem::path input(path);
        if (input.is_absolute() && bgui::resource_files::exists(input))
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
            if (!candidate.empty() && bgui::resource_files::exists(candidate))
                return candidate;
        }

        throw std::runtime_error("[AssetManager] Could not find asset: " + path);
    }

#ifdef _WIN32
    using Microsoft::WRL::ComPtr;

    bgui::texture decode_memory(const std::vector<unsigned char>& bytes, const std::string& name) {
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
        ComPtr<IWICStream> stream;
        result = factory->CreateStream(&stream);
        if (bytes.size() > MAXDWORD) throw std::runtime_error("Image is too large.");
        if (SUCCEEDED(result)) result = stream->InitializeFromMemory(const_cast<BYTE*>(bytes.data()), static_cast<DWORD>(bytes.size()));
        if (SUCCEEDED(result)) result = factory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnLoad, &decoder);

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
        loaded.m_path = name;
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
            throw std::runtime_error("[AssetManager] Could not decode asset: " + name);
        loaded.m_revision = 1;
        return loaded;
    }
#else
    struct jpeg_failure {
        jpeg_error_mgr manager{};
        std::jmp_buf jump;
        char message[JMSG_LENGTH_MAX]{};
    };

    void jpeg_error(j_common_ptr decoder) {
        auto* failure = reinterpret_cast<jpeg_failure*>(decoder->err);
        decoder->err->format_message(decoder, failure->message);
        std::longjmp(failure->jump, 1);
    }

    // All mutable resources live on the heap before setjmp. A libjpeg error must
    // not jump past the construction/destruction of C++ containers or handles.
    struct jpeg_state {
        jpeg_decompress_struct decoder{};
        jpeg_failure failure{};
        FILE* file = nullptr;
        bool created = false;
        bgui::texture loaded;
        std::vector<unsigned char> row;
        ~jpeg_state() {
            if (created) jpeg_destroy_decompress(&decoder);
            if (file) std::fclose(file);
        }
    };

    bgui::texture decode_jpeg(const std::vector<unsigned char>& bytes, const std::string& name) {
        auto state = std::make_unique<jpeg_state>();
        state->decoder.err = jpeg_std_error(&state->failure.manager);
        state->failure.manager.error_exit = jpeg_error;
        if (setjmp(state->failure.jump))
            throw std::runtime_error("[AssetManager] Could not decode JPEG: " + std::string(state->failure.message));
        state->created = true;
        jpeg_create_decompress(&state->decoder);
        jpeg_mem_src(&state->decoder, bytes.data(), static_cast<unsigned long>(bytes.size()));
        jpeg_read_header(&state->decoder, TRUE);
        const bool cmyk = state->decoder.jpeg_color_space == JCS_CMYK || state->decoder.jpeg_color_space == JCS_YCCK;
        state->decoder.out_color_space = cmyk ? JCS_CMYK : JCS_RGB;
        jpeg_start_decompress(&state->decoder);
        const auto width = static_cast<size_t>(state->decoder.output_width);
        const auto height = static_cast<size_t>(state->decoder.output_height);
        if (!width || !height || width > std::numeric_limits<size_t>::max() / height / 4)
            throw std::runtime_error("[AssetManager] Invalid JPEG dimensions");
        state->loaded.m_path = name;
        state->loaded.m_size = {static_cast<float>(width), static_cast<float>(height)};
        state->loaded.m_has_alpha = true; // RGBA storage, matching the PNG/WIC contract.
        state->loaded.m_buffer.resize(width * height * 4);
        state->row.resize(width * state->decoder.output_components);
        while (state->decoder.output_scanline < height) {
            const auto y = state->decoder.output_scanline;
            JSAMPROW row = state->row.data();
            jpeg_read_scanlines(&state->decoder, &row, 1);
            for (size_t x = 0; x < width; ++x) {
                auto* rgba = state->loaded.m_buffer.data() + (y * width + x) * 4;
                const auto* pixel = state->row.data() + x * state->decoder.output_components;
                for (size_t channel = 0; channel < 3; ++channel) {
                    if (!cmyk) rgba[channel] = pixel[channel];
                    else if (state->decoder.saw_Adobe_marker)
                        rgba[channel] = (static_cast<unsigned>(pixel[channel]) * pixel[3] + 127) / 255;
                    else rgba[channel] = ((255u - pixel[channel]) * (255u - pixel[3]) + 127) / 255;
                }
                rgba[3] = 255;
            }
        }
        jpeg_finish_decompress(&state->decoder);
        state->loaded.m_revision = 1;
        return std::move(state->loaded);
    }

    bgui::texture decode_memory(const std::vector<unsigned char>& bytes, const std::string& name) {
        if (bytes.size() >= 2 && bytes[0] == 0xff && bytes[1] == 0xd8) return decode_jpeg(bytes, name);

        png_image image{};
        image.version = PNG_IMAGE_VERSION;
        if (!png_image_begin_read_from_memory(&image, bytes.data(), bytes.size()))
            throw std::runtime_error("[AssetManager] Could not decode asset: " + name);

        image.format = PNG_FORMAT_RGBA;
        bgui::texture loaded;
        loaded.m_path = name;
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
        loaded.m_revision = 1;
        return loaded;
    }
#endif
}

namespace bgui {
    texture decode_texture_memory(const std::vector<unsigned char>& bytes, const std::string& name) {
        return decode_memory(bytes, name);
    }
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

        auto [it, inserted] = m_textures.emplace(resolved_path, decode_texture_memory(resource_files::read(resolved_path), resolved_path));
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
