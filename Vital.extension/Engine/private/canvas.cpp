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
        for (const auto &cmd : queue) {
            switch (cmd.type) {

                case Type::IMAGE: {
                    const auto &p = std::get<ImageCommand>(cmd.payload);

                    draw_set_transform(
                        p.rect.position + p.pivot,
                        p.rotation,
                        godot::Vector2(1, 1)
                    );

                    draw_texture_rect(
                        p.texture,
                        godot::Rect2(-p.pivot, p.rect.size),
                        false,
                        p.color
                    );
                } break;

                case Type::TEXT: {
                    const auto &p = std::get<TextCommand>(cmd.payload);

                    draw_multiline_string(
                        p.font,
                        p.rect.position,
                        p.text,
                        p.align_x,
                        p.wordwrap ? p.rect.size.x : -1,
                        p.font_size,
                        -1,
                        p.color
                    );
                } break;
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
        vp->set_clear_mode(godot::SubViewport::CLEAR_MODE_ALWAYS);

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

    void Singleton::dx_set_rendertarget(RenderTarget *rt, bool clear) {
        current_rt = rt;

        if (!rt)
            return;

        rt->viewport->set_clear_mode(
            clear
                ? godot::SubViewport::CLEAR_MODE_ALWAYS
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
        for (const auto &cmd : queue) {
            switch (cmd.type) {

                case Type::IMAGE: {
                    const auto &p = std::get<ImageCommand>(cmd.payload);

                    draw_set_transform(
                        p.rect.position + p.pivot,
                        p.rotation,
                        godot::Vector2(1, 1)
                    );

                    draw_texture_rect(
                        p.texture,
                        godot::Rect2(-p.pivot, p.rect.size),
                        false,
                        p.color
                    );
                } break;

                case Type::TEXT: {
                    const auto &p = std::get<TextCommand>(cmd.payload);

                    draw_multiline_string(
                        p.font,
                        p.rect.position,
                        p.text,
                        p.align_x,
                        p.wordwrap ? p.rect.size.x : -1,
                        p.font_size,
                        -1,
                        p.color
                    );
                } break;
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

        ImageCommand cmd;
        cmd.texture = texture;
        cmd.rect = { x, y, w, h };
        cmd.rotation = godot::Math::deg_to_rad(rotation);
        cmd.pivot = { pivot_x, pivot_y };
        cmd.color = color;

        if (current_rt) {
            auto *rtc = static_cast<RTCanvas*>(current_rt->canvas);
            rtc->queue.push_back({ Type::IMAGE, cmd });
            rtc->queue_redraw();
        } else {
            queue.push_back({ Type::IMAGE, cmd });
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
        draw_image(rt->texture, x, y, w, h, rotation, pivot_x, pivot_y, color);
    }

    void Singleton::draw_text(
        const godot::String &text,
        float left_x, float top_y,
        float right_x, float bottom_y,
        const godot::Ref<godot::Font> &font,
        int font_size,
        const godot::Color &color,
        godot::HorizontalAlignment align_x,
        godot::VerticalAlignment align_y,
        bool clip,
        bool wordwrap,
        float rotation,
        float pivot_x, float pivot_y
    ) {
        if (!font.is_valid() || text.is_empty()) return;

        TextCommand cmd;
        cmd.text = text;
        cmd.font = font;
        cmd.font_size = font_size;
        cmd.color = color;
        cmd.align_x = align_x;
        cmd.align_y = align_y;
        cmd.wordwrap = wordwrap;
        cmd.rect = { left_x, top_y, right_x - left_x, bottom_y - top_y };

        if (current_rt) {
            auto *rtc = static_cast<RTCanvas*>(current_rt->canvas);
            rtc->queue.push_back({ Type::TEXT, cmd });
            rtc->queue_redraw();
        } else {
            queue.push_back({ Type::TEXT, cmd });
        }
    }

} // namespace Vital::Godot::Canvas
