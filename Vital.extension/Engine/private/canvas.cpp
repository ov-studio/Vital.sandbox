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
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot::Engine {
    // Instantiators //    
    void Canvas::_ready() {
        queue.reserve(256);
        set_z_index(9999);
        set_visible(true);
        set_as_top_level(true);
        set_position(godot::Vector2(0, 0));
        set_rotation(0);
        set_scale(godot::Vector2(1, 1));
    }
    
    void Canvas::_clean() {
        queue.clear();
        cache.clear();
    }

    void Canvas::_process(double delta) {
        set_position(get_viewport() -> get_visible_rect().position);
        queue_redraw();
    }

    void Canvas::_draw() {
        for (const CanvasCommand& cmd : queue) {
            switch (cmd.type) {
                case CanvasType::IMAGE: {
                    godot::Vector2 center = cmd.rect.size*0.5f;
                    godot::Vector2 pivot = center + cmd.pivot;
                    draw_set_transform(cmd.rect.position + pivot, cmd.rotation, godot::Vector2(1, 1));
                    draw_texture_rect(
                        cmd.texture,
                        godot::Rect2(-pivot, cmd.rect.size),
                        false,
                        cmd.color
                    );
                    break;
                }
                case CanvasType::TEXT: {
                    draw_string(
                        cmd.font,
                        cmd.position,
                        cmd.text,
                        godot::HORIZONTAL_ALIGNMENT_LEFT,
                        -1,
                        cmd.font_size,
                        cmd.color
                    );
                    break;
                }
            }
        }
        _clean();
        Vital::Godot::Sandbox::Lua::Singleton::fetch() -> draw(this);
    }


    // Utils //
    godot::Ref<godot::Texture2D> Canvas::get_texture_from_path(const std::string& path) {
        auto it = cache.find(path);
        if (it != cache.end())
            return it->second;
    
        godot::Ref<godot::Texture2D> tex =
            godot::ResourceLoader::get_singleton()
                ->load(path.c_str(), "Texture2D");
    
        if (!tex.is_valid()) {
            godot::UtilityFunctions::printerr("dxDrawImage: failed to load ", path.c_str());
            return nullptr;
        }
    
        cache[path] = tex;
        return tex;
    }

    void Canvas::drawImage(
        const std::string& path,
        float x, float y,
        float w, float h,
        float rotation,
        float pivot_x,
        float pivot_y,
        const godot::Color& color
    ) {
        godot::Ref<godot::Texture2D> texture = get_texture_from_path(path);
        if (!texture.is_valid()) return;
        CanvasCommand cmd;
        cmd.type = CanvasType::IMAGE;
        cmd.texture = texture;
        cmd.rect = godot::Rect2(x, y, w, h);
        cmd.rotation = godot::Math::deg_to_rad(rotation);
        cmd.pivot = godot::Vector2(pivot_x, pivot_y);
        cmd.color = color;
    
        queue.push_back(cmd);
    }

    void Canvas::drawImage(
        const godot::Ref<godot::Texture2D>& texture,
        float x, float y,
        float w, float h,
        float rotation,
        float pivot_x,
        float pivot_y,
        const godot::Color& color
    ) {
        if (!texture.is_valid()) return;
        CanvasCommand cmd;
        cmd.type = CanvasType::IMAGE;
        cmd.texture = texture;
        cmd.rect = godot::Rect2(x, y, w, h);
        cmd.rotation = godot::Math::deg_to_rad(rotation);
        cmd.pivot = godot::Vector2(pivot_x, pivot_y);
        cmd.color = color;
        queue.push_back(cmd);
    }
    
    void Canvas::drawText(
        const godot::String& text,
        float x, float y,
        const godot::Ref<godot::Font>& font,
        int font_size,
        const godot::Color& color
    ) {
        if (!font.is_valid()) return;
        CanvasCommand cmd;
        cmd.type = CanvasType::TEXT;
        cmd.text = text;
        cmd.position = godot::Vector2(x, y);
        cmd.font = font;
        cmd.font_size = font_size;
        cmd.color = color;
        queue.push_back(cmd);
    }
}