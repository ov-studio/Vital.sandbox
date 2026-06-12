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
#if defined(VSDK_Client)
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
    Texture* Texture::get_from_reference(const std::string& reference) {
        auto it = reference_cache.find(reference);
        return it != reference_cache.end() ? it -> second : nullptr;
    }

    godot::Ref<godot::Texture2D> Texture::get_texture() const {
        switch (command.type) {
            case Type::Texture2D: return std::get<Texture2D>(command.payload).texture;
            case Type::SVG: return std::get<SVG>(command.payload).texture;
        }
        return godot::Ref<godot::Texture2D>();
    }

    godot::Vector2i Texture::get_size() const {
        auto texture = get_texture();
        if (!texture.is_valid()) return godot::Vector2i();
        return texture -> get_size();
    }


    // APIs //
    Texture* Texture::create_texture_2d(const std::string& path, const std::string& reference) {
        return create_texture_2d_from_buffer(Tool::File::read_binary(Tool::get_directory(), path), reference);
    }

    Texture* Texture::create_texture_2d_from_buffer(const godot::PackedByteArray& buffer, const std::string& reference) {
        godot::Ref<godot::Image> image;
        image.instantiate();
        godot::Error status;
        switch (Tool::Format::get_format(format_registry, Format::UNKNOWN, buffer)) {
            case Format::JPG: status = image -> load_jpg_from_buffer(buffer); break;
            case Format::PNG: status = image -> load_png_from_buffer(buffer); break;
            case Format::WEBP: status = image -> load_webp_from_buffer(buffer); break;
            case Format::BMP: status = image -> load_bmp_from_buffer(buffer); break;
            case Format::DDS: status = image -> load_dds_from_buffer(buffer); break;
            case Format::KTX: status = image -> load_ktx_from_buffer(buffer); break;
            default: break;
        }
        if (status != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid texture buffer");
        Texture2D payload;
        payload.texture = godot::ImageTexture::create_from_image(image);
        return memnew(Texture({Type::Texture2D, payload}, reference));
    }

    Texture* Texture::create_svg(const std::string& path, const std::string& reference) {
        return create_svg_from_buffer(Tool::File::read_binary(Tool::get_directory(), path), reference);
    }

    Texture* Texture::create_svg_from_raw(const std::string& raw, const std::string& reference) {
        godot::Ref<godot::Image> image;
        image.instantiate();
        if (image -> load_svg_from_string(Tool::to_godot_string(raw), 1.0) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid svg buffer");
        SVG payload;
        payload.texture = godot::ImageTexture::create_from_image(image);
        return memnew(Texture({Type::SVG, payload}, reference));
    }

    Texture* Texture::create_svg_from_buffer(const godot::PackedByteArray& buffer, const std::string& reference) {
        godot::Ref<godot::Image> image;
        image.instantiate();
        if (image -> load_svg_from_buffer(buffer, 1.0) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid svg buffer");
        SVG payload;
        payload.texture = godot::ImageTexture::create_from_image(image);
        return memnew(Texture({Type::SVG, payload}, reference));
    }

    void Texture::update_svg_from_raw(const std::string& raw) {
        if (command.type != Type::SVG) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid command type");
        godot::Ref<godot::Image> image;
        image.instantiate();
        if (image -> load_svg_from_string(Tool::to_godot_string(raw), 1.0) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid svg buffer");
        std::get<SVG>(command.payload).texture -> update(image);
        heartbeat();
    }

    void Texture::update_svg_from_buffer(const godot::PackedByteArray& buffer) {
        if (command.type != Type::SVG) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid command type");
        godot::Ref<godot::Image> image;
        image.instantiate();
        if (image -> load_svg_from_buffer(buffer, 1.0) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid svg buffer");
        std::get<SVG>(command.payload).texture -> update(image);
        heartbeat();
    }
        heartbeat();
    }
}
#endif