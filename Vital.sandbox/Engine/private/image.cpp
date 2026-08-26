/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: image.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Image Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/image.h>


///////////////////////////
// Vital: Engine: Image //
///////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Image::Image(const godot::Ref<godot::ImageTexture>& texture, const std::string& reference) : Texture(reference), texture(texture) {}


    // Checkers //
    bool Image::is_compressed() const {
        auto texture = get_texture();
        if (!texture.is_valid()) return false;
        return texture -> get_image() -> is_compressed();
    }


    // Misc //
    Image* Image::create_texture_2d(const std::string& base, const std::string& path, bool mipmaps, const std::string& reference) {
        return create_texture_2d_from_buffer(Tool::File::read_binary(base, path), mipmaps, reference);
    }

    Image* Image::create_texture_2d_from_buffer(const godot::PackedByteArray& buffer, bool mipmaps, const std::string& reference) {
        godot::Ref<godot::Image> image;
        image.instantiate();
        godot::Error status;
        switch (Tool::Format::get_format(format_registry, Format::UNKNOWN, buffer)) {
            case Format::JPG:  status = image -> load_jpg_from_buffer(buffer);  break;
            case Format::PNG:  status = image -> load_png_from_buffer(buffer);  break;
            case Format::WEBP: status = image -> load_webp_from_buffer(buffer); break;
            case Format::BMP:  status = image -> load_bmp_from_buffer(buffer);  break;
            case Format::DDS:  status = image -> load_dds_from_buffer(buffer);  break;
            case Format::KTX:  status = image -> load_ktx_from_buffer(buffer);  break;
            case Format::EXR:  status = image -> load_exr_from_buffer(buffer);  break;
            default: break;
        }
        if (status != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "invalid texture buffer");
        if (mipmaps && image -> generate_mipmaps() != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "failed to generate mipmaps");
        return memnew(Image(godot::ImageTexture::create_from_image(image), reference));
    }

    void Image::convert(godot::Image::Format format) {
        auto image = get_texture() -> get_image();
        image -> convert(format);
        get_texture() -> update(image);
        heartbeat();
    }

    void Image::compress(godot::Image::CompressMode mode) {
        auto image = get_texture() -> get_image();
        if (is_compressed()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "texture is already compressed");
        if (image -> compress(mode, godot::Image::COMPRESS_SOURCE_GENERIC) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "compression failed");
        get_texture() -> update(image);
        heartbeat();
    }
}
#endif
