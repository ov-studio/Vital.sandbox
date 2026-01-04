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
        set_z_index(godot::RenderingServer::CANVAS_ITEM_Z_MAX);
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
                    const TextCommand &payload = std::get<TextCommand>(command.payload);
                    godot::Vector2 center = payload.rect.size*0.5f;
                    godot::Vector2 pivot = center + payload.pivot;
                    //draw_set_transform(payload.rect.position + pivot, payload.rotation, godot::Vector2(1, 1));


                    // -----------------------------
                    // 1. Rotation (optional)
                    // -----------------------------
                    if (payload.rotation != 0.0f) {
                        draw_set_transform(
                            payload.rect.position + payload.pivot,
                            payload.rotation,
                            godot::Vector2(1, 1)
                        );
                    }
                    else {
                        draw_set_transform(
                            godot::Vector2(),
                            0.0f,
                            godot::Vector2(1, 1)
                        );
                    }
                
                    draw_multiline_string(
                        payload.font,
                        payload.rect.position,
                        payload.text,
                        payload.align_x,
                        payload.rect.size.x,
                        payload.font_size
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


    // APIs //
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
        float left_x, float top_y,
        float right_x, float bottom_y,
        const godot::Ref<godot::Font>& font,
        int font_size,
        const godot::Color& color,
        godot::HorizontalAlignment alignment_x, godot::VerticalAlignment alignment_y,
        bool clip,
        bool wordwrap,
        float rotation,
        float pivot_x, float pivot_y
    ) {
        if (!font.is_valid() || text.is_empty()) return;
        TextCommand payload;
        payload.text = text;
        payload.rect = godot::Rect2(left_x, top_y, right_x - left_x, bottom_y - top_y);
        payload.position = godot::Vector2(left_x, top_y);
        payload.rotation = godot::Math::deg_to_rad(rotation);
        payload.font = font;
        payload.font_size = font_size;
        payload.font_ascent = payload.font -> get_ascent(payload.font_size);
        payload.color = color;
        payload.align_x = alignment_x;
        payload.align_y = alignment_y;
        payload.pivot = godot::Vector2(pivot_x, pivot_y);
        payload.clip = clip;
        payload.wordwrap = wordwrap;
        payload.rect.position.y += payload.font_ascent;
        payload.text_size = payload.font -> get_multiline_string_size(
            payload.text,
            payload.align_x,
            payload.wordwrap ? payload.rect.size.x : -1,
            payload.font_size
        );
        payload.rect.size.y = payload.wordwrap ? payload.text_size.y : payload.rect.size.y;
        payload.text_lines = payload.rect.size.y/(payload.text_size.y + payload.font_ascent);
        if (payload.align_y == godot::VERTICAL_ALIGNMENT_CENTER) payload.rect.position.y += (payload.rect.size.y - payload.text_size.y)*0.5f;
        else if (payload.align_y == godot::VERTICAL_ALIGNMENT_BOTTOM) payload.rect.position.y += payload.rect.size.y - payload.text_size.y;
        queue.push_back(Command{Type::TEXT, std::move(payload)});
    }
}