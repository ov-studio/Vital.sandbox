/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: svg.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SVG Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/svg.h>


/////////////////////////
// Vital: Engine: SVG //
/////////////////////////

namespace Vital::Engine {
    // Instantiators //
    SVG::SVG(const godot::Ref<godot::ImageTexture>& texture, const std::string& reference) : Texture(reference), texture(texture) {}


    // Misc //
    SVG* SVG::create(const std::string& base, const std::string& path, bool mipmaps, const std::string& reference) {
        return create_from_buffer(Tool::File::read_binary(base, path), mipmaps, reference);
    }

    SVG* SVG::create_from_raw(const std::string& raw, bool mipmaps, const std::string& reference) {
        godot::Ref<godot::Image> image;
        image.instantiate();
        if (image -> load_svg_from_string(Tool::to_godot_string(raw), 1.0) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "invalid svg buffer");
        if (mipmaps && image -> generate_mipmaps() != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "failed to generate mipmaps");
        return memnew(SVG(godot::ImageTexture::create_from_image(image), reference));
    }

    SVG* SVG::create_from_buffer(const godot::PackedByteArray& buffer, bool mipmaps, const std::string& reference) {
        godot::Ref<godot::Image> image;
        image.instantiate();
        if (image -> load_svg_from_buffer(buffer, 1.0) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "invalid svg buffer");
        if (mipmaps && image -> generate_mipmaps() != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "failed to generate mipmaps");
        return memnew(SVG(godot::ImageTexture::create_from_image(image), reference));
    }

    void SVG::update_from_raw(const std::string& raw) {
        godot::Ref<godot::Image> image;
        image.instantiate();
        if (image -> load_svg_from_string(Tool::to_godot_string(raw), 1.0) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "invalid svg buffer");
        get_texture() -> update(image);
        heartbeat();
    }

    void SVG::update_from_buffer(const godot::PackedByteArray& buffer) {
        godot::Ref<godot::Image> image;
        image.instantiate();
        if (image -> load_svg_from_buffer(buffer, 1.0) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "invalid svg buffer");
        get_texture() -> update(image);
        heartbeat();
    }
}
#endif
