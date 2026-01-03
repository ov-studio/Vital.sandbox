/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: canvas.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Canvas Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/canvas.h>
#include <Vital.extension/Sandbox/lua/public/index.h>


///////////////////////////
// Vital: Godot: Canvas //
///////////////////////////

namespace Vital::Godot::Canvas {
    // Instantiators //    
    void Singleton::_ready() {
        queue.reserve(256);
        set_z_index(9999);
        set_visible(true);
        set_as_top_level(true);
        set_position(godot::Vector2(0, 0));
        set_rotation(0);
        set_scale(godot::Vector2(1, 1));
    }
    
    void Singleton::_clean() {
        queue.clear();
        textures.clear();
    }

    void Singleton::_process(double delta) {
        set_position(get_viewport() -> get_visible_rect().position);
        queue_redraw();
    }

    void Singleton::_draw() {
        for (const auto& command : queue) {
            switch (command.type) {
                case Type::IMAGE: {
                    const auto &payload = std::get<ImageCommand>(command.payload);
                    godot::Vector2 center = payload.rect.size*0.5f;
                    godot::Vector2 pivot = center + payload.pivot;
                    draw_set_transform(payload.rect.position + pivot, payload.rotation, godot::Vector2(1, 1));
                    draw_texture_rect(
                        payload.texture,
                        godot::Rect2(-pivot, payload.rect.size),
                        false,
                        payload.color
                    );
                    break;
                }
                case Type::TEXT: {
                    const auto &payload = std::get<TextCommand>(command.payload);
                    draw_string(
                        payload.font,
                        payload.position,
                        payload.text,
                        godot::HORIZONTAL_ALIGNMENT_LEFT,
                        -1,
                        payload.font_size,
                        payload.color
                    );
                    break;
                }
            }
        }
        _clean();
        Vital::Godot::Sandbox::Lua::Singleton::fetch() -> draw(this);
    }


    // Utils //
    godot::Ref<godot::Texture2D> Singleton::fetch_texture(const std::string& path) {
        auto match = textures.find(path);
        if (match != textures.end()) return match -> second;
        godot::Ref<godot::Texture2D> tex = godot::ResourceLoader::get_singleton() -> load(path.c_str(), "Texture2D");
        if (!tex.is_valid()) return nullptr;    
        textures.emplace(path, tex);
        return tex;
    }

    void Singleton::draw_image(
        const godot::Ref<godot::Texture2D>& texture,
        float x, float y,
        float w, float h,
        float rotation,
        float pivot_x, float pivot_y,
        const godot::Color& color
    ) {
        if (!texture.is_valid()) return;
        ImageCommand payload;
        payload.texture = texture;
        payload.rect = godot::Rect2(x, y, w, h);
        payload.rotation = godot::Math::deg_to_rad(rotation);
        payload.pivot = godot::Vector2(pivot_x, pivot_y);
        payload.color = color;
        queue.push_back(Command {Type::IMAGE, std::move(payload)});
    }

    void Singleton::draw_image(
        const std::string& path, 
        float x, float y, 
        float w, float h, 
        float rotation, 
        float pivot_x, float pivot_y, 
        const godot::Color& color
    ) {
        draw_image(fetch_texture(path), x, y, w, h, rotation, pivot_x, pivot_y, color);
    }

    void Singleton::draw_text(
        const godot::String& text,
        float x, float y,
        const godot::Ref<godot::Font>& font,
        int font_size,
        const godot::Color& color
    ) {
        if (!font.is_valid()) return;
        TextCommand payload;
        payload.text = text;
        payload.position = godot::Vector2(x, y);
        payload.font = font;
        payload.font_size = font_size;
        payload.color = color;
        queue.push_back(Command {Type::TEXT, std::move(payload)});
    }
}