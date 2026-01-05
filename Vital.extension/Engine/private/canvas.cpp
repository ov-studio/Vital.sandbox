/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: canvas.cpp
     Desc: Canvas Utilities (Godot 4 – FIXED RenderTargets)
----------------------------------------------------------------*/

#include <Vital.extension/Engine/public/canvas.h>
#include <Vital.extension/Sandbox/lua/public/index.h>

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/math.hpp>

namespace Vital::Godot::Canvas {

    void RTCanvas::_draw() {
        for (const auto &command : queue) {
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
    }


    // ============================================================
    // LIFECYCLE
    // ============================================================
    void Singleton::_ready() {
        queue.reserve(256);
        set_as_top_level(true);
        set_visible(true);
        set_z_index(godot::RenderingServer::CANVAS_ITEM_Z_MAX);
    }

    void Singleton::_process(double) {
        queue_redraw();
    }

    // ============================================================
    // RENDER TARGET API
    // ============================================================
    RenderTarget* Singleton::dx_create_rendertarget(int w, int h, bool transparent) {
        RenderTarget *rt = memnew(RenderTarget);

        godot::SubViewport *vp = memnew(godot::SubViewport);
        vp->set_size({ w, h });
        vp->set_disable_3d(true);
        vp->set_transparent_background(transparent);

        // 🔥 REQUIRED
        vp->set_update_mode(
            godot::SubViewport::UPDATE_ALWAYS
        );

        add_child(vp);

        RTCanvas *canvas = memnew(RTCanvas);
        vp->add_child(canvas);

        rt->viewport = vp;
        rt->canvas = canvas;
        rt->texture = vp->get_texture();
        rt->size = { w, h };

        owned_rts.push_back(rt);
        return rt;
    }

    void Singleton::dx_set_rendertarget(RenderTarget *rt, bool clear, bool reload) {
        current_rt = rt;

        if (!rt)
            return;

        rt->viewport->set_clear_mode(
            clear
                ? godot::SubViewport::CLEAR_MODE_ONCE
                : godot::SubViewport::CLEAR_MODE_NEVER
        );

        if (clear) {
            static_cast<RTCanvas*>(rt->canvas)->clear();
            rt->canvas->queue_redraw();
        }
    }

    // ============================================================
    // SCREEN DRAW
    // ============================================================
    void Singleton::_draw() {
        for (const auto &command : queue) {
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

        queue.clear();

        Vital::Godot::Sandbox::Lua::Singleton::fetch()->draw(this);
    }

    // ============================================================
    // UTILS
    // ============================================================
    godot::Ref<godot::Texture2D> Singleton::fetch_texture(const std::string &path) {
        auto it = textures.find(path);
        if (it != textures.end())
            return it->second;

        auto tex = godot::ResourceLoader::get_singleton()
            ->load(path.c_str(), "Texture2D");

        if (tex.is_valid())
            textures[path] = tex;

        return tex;
    }

    // ============================================================
    // DRAW APIs
    // ============================================================
    void Singleton::draw_image(
        const godot::Ref<godot::Texture2D> &texture,
        float x, float y,
        float w, float h,
        float rotation,
        float pivot_x, float pivot_y,
        const godot::Color &color
    ) {
        if (!texture.is_valid()) return;

        ImageCommand payload;
        payload.texture = texture;
        payload.rect = { x, y, w, h };
        payload.rotation = godot::Math::deg_to_rad(rotation);
        payload.pivot = { pivot_x, pivot_y };
        payload.color = color;

        if (current_rt) {
            auto *rtc = static_cast<RTCanvas*>(current_rt->canvas);
            rtc->queue.push_back({ Type::IMAGE, payload });
            rtc->queue_redraw();
        } else {
            queue.push_back({ Type::IMAGE, payload });
        }
    }

    void Singleton::draw_image(
        const std::string &path,
        float x, float y,
        float w, float h,
        float rotation,
        float pivot_x, float pivot_y,
        const godot::Color &color
    ) {
        draw_image(fetch_texture(path), x, y, w, h, rotation, pivot_x, pivot_y, color);
    }

    void Singleton::draw_image(
        RenderTarget *rt,
        float x, float y,
        float w, float h,
        float rotation,
        float pivot_x, float pivot_y,
        const godot::Color &color
    ) {
        if (!rt) return;
        draw_image(rt -> texture, x, y, w, h, rotation, pivot_x, pivot_y, color);
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

        if (current_rt) {
            auto *rtc = static_cast<RTCanvas*>(current_rt->canvas);
            rtc->queue.push_back({ Type::TEXT, payload });
            rtc->queue_redraw();
        } else {
            queue.push_back({ Type::TEXT, payload });
        }
    }
}
