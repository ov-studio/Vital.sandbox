/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: texture.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Texture Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#include <Vital.extension/Engine/public/texture.h>


////////////////////////////
// Vital: Godot: Texture //
////////////////////////////

namespace Vital::Godot {
    // Getters //
    Texture::Format Texture::get_format(godot::PackedByteArray buffer) {
        const uint8_t* ptr = buffer.ptr();
        const int size = buffer.size();
        godot::Ref<godot::Image> image;
        image.instantiate();
        godot::Error status;
        if (
            size >= 8 &&
            ptr[0] == 0x89 && ptr[1] == 0x50 &&
            ptr[2] == 0x4E && ptr[3] == 0x47 &&
            ptr[4] == 0x0D && ptr[5] == 0x0A &&
            ptr[6] == 0x1A && ptr[7] == 0x0A
        ) return Format::PNG;
        else if (
            size >= 2 &&
            ptr[0] == 0xFF && ptr[1] == 0xD8
        ) return Format::JPG;
        else if (
            size >= 12 &&  
            ptr[0] == 'R' && ptr[1] == 'I' &&
            ptr[2] == 'F' && ptr[3] == 'F' &&
            ptr[8] == 'W' && ptr[9] == 'E' &&
            ptr[10] == 'B' && ptr[11] == 'P'
        ) return Format::WEBP;
        return Format::UNKNOWN;
    }

    godot::Ref<godot::Texture2D> Texture::get_texture() {
        switch (command.type) {
            case Type::Texture2D: {
                const auto& payload = std::get<Texture2D>(command.payload);
                return payload.texture;
            }
            case Type::SVG: {
                const auto& payload = std::get<SVG>(command.payload);
                return payload.texture;
            }
        }
        return godot::Ref<godot::Texture2D>();
    }


    // APIs //
    Texture* Texture::create_texture_2d(const std::string& path) {
        godot::Ref<godot::FileAccess> file = godot::FileAccess::open(path.c_str(), godot::FileAccess::READ);
        if (file.is_null()) throw std::runtime_error(ErrorCode["invalid-arguments"]);
        const int64_t size = file->get_length();
        if (size < 4) throw std::runtime_error(ErrorCode["invalid-arguments"]);
        godot::PackedByteArray buffer = file -> get_buffer(size);
        godot::Ref<godot::Image> image;
        image.instantiate();
        godot::Error status;
        switch (get_format(buffer)) {
            case Format::PNG: {
                status = image -> load_png_from_buffer(buffer);
                break;
            }
            case Format::JPG: {
                status = image -> load_jpg_from_buffer(buffer);
                break;
            }
            case Format::WEBP: {
                status = image -> load_webp_from_buffer(buffer);
                break;
            }
        }
        if (status != godot::OK) throw std::runtime_error(ErrorCode["invalid-arguments"]);
        Texture2D payload;
        auto* texture = memnew(Texture);
        payload.texture = godot::ImageTexture::create_from_image(image);
        texture -> command = {Type::Texture2D, payload};
        return texture;
    }

    Texture* Texture::create_svg(const std::string& path) {
        auto buffer = godot::FileAccess::get_file_as_string(path.c_str());
        if (buffer.is_empty()) throw std::runtime_error(ErrorCode["invalid-arguments"]);
        godot::Ref<godot::Image> image;
        image.instantiate();
        godot::Error status = image -> load_svg_from_string(buffer, 1.0);
        if (status != godot::OK) throw std::runtime_error(ErrorCode["invalid-arguments"]);
        SVG payload;
        auto* texture = memnew(Texture);
        payload.texture = godot::ImageTexture::create_from_image(image);
        texture -> command = {Type::SVG, payload};
        return texture;
    }
}