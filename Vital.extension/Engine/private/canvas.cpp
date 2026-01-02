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
    }

    void Canvas::_process(double delta) {
        set_position(get_viewport() -> get_visible_rect().position);
        queue_redraw();
    }

    void Canvas::_draw() {
        for (const DrawCommand& cmd : queue) {
            switch (cmd.type) {
                case DrawType::IMAGE: {    
                    draw_texture_rect(
                        cmd.texture,
                        cmd.rect,
                        false,
                        cmd.color
                    );
                    break;
                }
                case DrawType::TEXT: {
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
    void Canvas::drawImage(
        const godot::Ref<godot::Texture2D>& texture,
        float x, float y, float w, float h,
        const godot::Color& color
    ) {
        if (!texture.is_valid()) return;
        DrawCommand cmd;
        cmd.type = DrawType::IMAGE;
        cmd.texture = texture;
        cmd.rect = godot::Rect2(x, y, w, h);
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
        DrawCommand cmd;
        cmd.type = DrawType::TEXT;
        cmd.text = text;
        cmd.position = godot::Vector2(x, y);
        cmd.font = font;
        cmd.font_size = font_size;
        cmd.color = color;
        queue.push_back(cmd);
    }
}