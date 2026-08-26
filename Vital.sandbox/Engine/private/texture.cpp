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
    Texture::Texture(const std::string& reference) {
        if (!reference.empty()) {
            reference_key = reference;
            reference_cache.emplace(reference_key, this);
            heartbeat();
        }
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
        for (const auto& key : expired) reference_cache[key] -> destroy();
    }


    // Checkers //
    bool Texture::has_mipmaps() const {
        auto texture = get_texture();
        if (!texture.is_valid()) return false;
        return texture -> get_image() -> has_mipmaps();
    }


    // Getters //
    Texture* Texture::get_from_reference(const std::string& reference) {
        auto it = reference_cache.find(reference);
        return it != reference_cache.end() ? it -> second : nullptr;
    }

    godot::Ref<godot::Texture2D> Texture::get_canvas_texture() const {
        if (canvas_texture.is_valid()) return canvas_texture;
        return get_texture();
    }

    godot::Vector2i Texture::get_size() const {
        auto texture = get_texture();
        if (!texture.is_valid()) return godot::Vector2i();
        return texture -> get_size();
    }

    godot::CanvasItem::TextureFilter Texture::get_filter() const {
        return canvas_texture.is_valid() ? canvas_texture -> get_texture_filter() : godot::CanvasItem::TEXTURE_FILTER_PARENT_NODE;
    }


    // Setters //
    void Texture::set_filter(godot::CanvasItem::TextureFilter mode) {
        if (mode == godot::CanvasItem::TEXTURE_FILTER_PARENT_NODE) canvas_texture.unref();
        else {
            auto texture = get_texture();
            bool wants_mipmaps = (
                mode == godot::CanvasItem::TEXTURE_FILTER_NEAREST_WITH_MIPMAPS ||
                mode == godot::CanvasItem::TEXTURE_FILTER_LINEAR_WITH_MIPMAPS ||
                mode == godot::CanvasItem::TEXTURE_FILTER_NEAREST_WITH_MIPMAPS_ANISOTROPIC ||
                mode == godot::CanvasItem::TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC
            );
            if (wants_mipmaps && !has_mipmaps()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "requested filter mode requires mipmaps, but texture has none");
            if (!canvas_texture.is_valid()) canvas_texture.instantiate();
            canvas_texture -> set_diffuse_texture(texture);
            canvas_texture -> set_texture_filter(mode);
        }
        heartbeat();
    }
}
#endif
