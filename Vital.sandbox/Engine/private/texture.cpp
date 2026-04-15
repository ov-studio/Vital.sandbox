/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: texture.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Texture Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Engine/public/texture.h>


/////////////////////////////
// Vital: Engine: Texture //
/////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Texture::Texture(Command cmd, const std::string& reference) {
        if (!reference.empty()) {
            reference_key = reference;
            reference_cache.emplace(reference_key, this);
            heartbeat();
        }
        command = cmd;
    }

    Texture::~Texture() {
        if (reference_key.empty()) return;
        reference_cache.erase(reference_key);
    }


    // Managers //
    void Texture::destroy() {
        memdelete(this);
    }

    void Texture::heartbeat() {
        reference_tick = Tool::get_tick();
    }

    void Texture::flush() {
        auto tick = Tool::get_tick();
        std::vector<std::string> expired;
        for (const auto& reference_cache : reference_cache) {
            if (tick - reference_cache.second -> reference_tick > flush_interval) {
                expired.push_back(reference_cache.first);
            }
        }
        for (const auto& key : expired) {
            reference_cache[key] -> destroy();
        }
    }


    // Getters //
    Texture::Format Texture::get_format(const godot::PackedByteArray& buffer) {
        const uint8_t* ptr = buffer.ptr();
        const int size = buffer.size();
        godot::Ref<godot::Image> image;
        image.instantiate();
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

    Texture* Texture::get_from_reference(const std::string& reference) {
        auto it = reference_cache.find(reference);
        return it != reference_cache.end() ? it -> second : nullptr;
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
    Texture* Texture::create_texture_2d(const std::string& path, const std::string& reference) {
        return create_texture_2d_from_buffer(Tool::File::read_binary(Tool::get_directory(), path), reference);
    }

    Texture* Texture::create_texture_2d_from_buffer(const godot::PackedByteArray& buffer, const std::string& reference) {
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
        if (status != godot::OK) throw Tool::Log::fetch("invalid-arguments", Tool::Log::Type::Error);
        Texture2D payload;
        auto texture = memnew(Texture({Type::Texture2D, payload}, reference));
        payload.texture = godot::ImageTexture::create_from_image(image);
        return texture;
    }

    Texture* Texture::create_svg(const std::string& path, const std::string& reference) {
        return create_svg_from_buffer(Tool::File::read_binary(Tool::get_directory(), path), reference);
    }

    Texture* Texture::create_svg_from_raw(const std::string& raw, const std::string& reference) {
        godot::Ref<godot::Image> image;
        image.instantiate();
        godot::Error status = image -> load_svg_from_string(Tool::to_godot_string(raw), 1.0);
        if (status != godot::OK) throw Tool::Log::fetch("invalid-arguments", Tool::Log::Type::Error);
        SVG payload;
        auto texture = memnew(Texture({Type::SVG, payload}, reference));
        payload.texture = godot::ImageTexture::create_from_image(image);
        return texture;
    }

    Texture* Texture::create_svg_from_buffer(const godot::PackedByteArray& buffer, const std::string& reference) {
        godot::Ref<godot::Image> image;
        image.instantiate();
        godot::Error status = image -> load_svg_from_buffer(buffer, 1.0);
        if (status != godot::OK) throw Tool::Log::fetch("invalid-arguments", Tool::Log::Type::Error);
        SVG payload;
        auto texture = memnew(Texture({Type::SVG, payload}, reference));
        payload.texture = godot::ImageTexture::create_from_image(image);
        return texture;
    }

    void Texture::update_svg_from_raw(const std::string& raw) {
        if (command.type != Type::SVG) throw Tool::Log::fetch("invalid-arguments", Tool::Log::Type::Error);
        godot::Ref<godot::Image> image;
        image.instantiate();
        godot::Error status = image -> load_svg_from_string(Tool::to_godot_string(raw), 1.0);
        if (status != godot::OK) throw Tool::Log::fetch("invalid-arguments", Tool::Log::Type::Error);
        const auto& payload = std::get<SVG>(command.payload);
        payload.texture -> update(image);
        heartbeat();
    }

    void Texture::update_svg_from_buffer(const godot::PackedByteArray& buffer) {
        if (command.type != Type::SVG) throw Tool::Log::fetch("invalid-arguments", Tool::Log::Type::Error);
        godot::Ref<godot::Image> image;
        image.instantiate();
        godot::Error status = image -> load_svg_from_buffer(buffer, 1.0);
        if (status != godot::OK) throw Tool::Log::fetch("invalid-arguments", Tool::Log::Type::Error);
        const auto& payload = std::get<SVG>(command.payload);
        payload.texture -> update(image);
        heartbeat();
    }
}
#endif