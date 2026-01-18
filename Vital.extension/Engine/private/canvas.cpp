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

#if defined(Vital_SDK_Client)
#pragma once
#include <Vital.extension/Engine/public/canvas.h>
#include <Vital.extension/Engine/public/texture.h>
#include <Vital.extension/Engine/public/rendertarget.h>
#include <Vital.extension/Sandbox/lua/public/index.h>


///////////////////////////
// Vital: Godot: Canvas //
///////////////////////////

namespace Vital::Godot {
    // Instantiators //
    void Canvas::_ready() {
        queue.reserve(256);
        set_as_top_level(true);
        set_visible(true);
        set_z_index(godot::RenderingServer::CANVAS_ITEM_Z_MAX);
    }

    void Canvas::_process(double) {
        queue_redraw();
        Texture::flush();
    }

    void Canvas::_clean() {
        queue.clear();
    }

    void Canvas::_draw() {
        Canvas::execute(static_cast<godot::Node2D*>(this), queue);
        _clean();
        Sandbox::Lua::Singleton::fetch() -> draw(this);
    }


    // Utils //
    Canvas* Canvas::get_singleton() {
        if (!singleton) {
            singleton = memnew(Canvas);
            Core::get_singleton() -> get_tree() -> get_root() -> call_deferred("add_child", singleton);
        }
        return singleton;
    }

    void Canvas::free_singleton() {
        if (!singleton) return
        singleton -> queue_free();
        singleton = nullptr;
    }

    void Canvas::execute(godot::Node2D* node, std::vector<Command> queue) {
        for (const auto &command : queue) {
            switch (command.type) {
                case Type::Line: {
                    const auto& payload = std::get<Line>(command.payload);
                    node -> draw_set_transform({0, 0}, 0, {1, 1});
                    node -> draw_polyline(
                        payload.points,
                        payload.color,
                        payload.stroke,
                        true
                    );
                    break;
                }
                case Type::Polygon: {
                    const auto& payload = std::get<Polygon>(command.payload);
                    node -> draw_set_transform(payload.rect.position + payload.pivot, payload.rotation, {1, 1});
                    if (payload.stroke > 0.0f) {
                        node -> draw_polyline(
                            payload.stroke_points,
                            payload.stroke_color,
                            payload.stroke,
                            true
                        );
                    }
                    node -> draw_colored_polygon(
                        payload.points,
                        payload.color
                    );
                    break;
                }
                case Type::Rectangle: {
                    const auto& payload = std::get<Rectangle>(command.payload);
                    auto pivot = payload.rect.size*0.5f + payload.pivot;
                    node -> draw_set_transform(payload.rect.position + pivot, payload.rotation, {1, 1});
                    if (payload.stroke > 0.0f) {
                        node -> draw_rect(
                            godot::Rect2(-pivot - godot::Vector2(payload.stroke*0.5, payload.stroke*0.5), payload.rect.size + godot::Vector2(payload.stroke, payload.stroke)),
                            payload.stroke_color,
                            false,
                            payload.stroke,
                            true
                        );
                    }
                    node -> draw_rect(
                        godot::Rect2(-pivot, payload.rect.size),
                        payload.color,
                        true,
                        -1,
                        true
                    );
                    break;
                }
                case Type::Circle: {
                    const auto& payload = std::get<Circle>(command.payload);
                    auto pivot = payload.pivot;
                    node -> draw_set_transform(payload.position + pivot, payload.rotation, {1, 1});
                    if (payload.stroke > 0.0f) {
                        node -> draw_circle(
                            -pivot,
                            payload.radius + payload.stroke*0.5,
                            payload.stroke_color,
                            false,
                            payload.stroke,
                            true
                        );
                    }
                    node -> draw_circle(
                        -pivot,
                        payload.radius,
                        payload.color,
                        true,
                        -1,
                        true
                    );
                    break;
                }
                case Type::IMAGE: {
                    const auto& payload = std::get<Image>(command.payload);
                    auto pivot = payload.rect.size*0.5f + payload.pivot;
                    node -> draw_set_transform(payload.rect.position + pivot, payload.rotation, {1, 1});
                    node -> draw_texture_rect(
                        payload.texture,
                        godot::Rect2(-pivot, payload.rect.size),
                        false,
                        payload.color
                    );
                    break;
                }
                case Type::TEXT: {
                    const auto& payload = std::get<Text>(command.payload);
                    auto pivot = payload.rect.size*0.5f + payload.pivot;
                    pivot.y -= payload.font_ascent;
                    node -> draw_set_transform(payload.rect.position + pivot, payload.rotation, {1, 1});
                    if (payload.stroke > 0) {
                        node -> draw_multiline_string_outline(
                            payload.font,
                            -pivot,
                            payload.text,
                            payload.alignment.first,
                            payload.rect.size.x,
                            payload.font_size,
                            -1,
                            payload.stroke,
                            payload.stroke_color
                        );
                    }
                    node -> draw_multiline_string(
                        payload.font,
                        -pivot,
                        payload.text,
                        payload.alignment.first,
                        payload.rect.size.x,
                        payload.font_size,
                        -1,
                        payload.color
                    );
                    break;
                }
            }
        }
    };

    void Canvas::push(Command command) {
        auto* rt = RenderTarget::get_rendertarget();
        if (rt) return rt -> push(command);
        queue.push_back(command);
    }


    // APIs //
    void Canvas::draw_line(
        godot::PackedVector2Array points,
        float stroke,
        const godot::Color& color
    ) {
        if (points.size() < 2) return;
        Line payload;
        payload.points = points;
        payload.stroke = stroke;
        payload.color = color;
        push({Type::Line, payload});
    }

    void Canvas::draw_polygon(
        godot::PackedVector2Array points,
        const godot::Color& color,
        float stroke,
        const godot::Color& stroke_color,
        float rotation,
        godot::Vector2 pivot
    ) {
        if (points.size() < 3) return;
        Polygon payload;
        godot::Vector2 min = points[0];
        godot::Vector2 max = points[0];
        for (int i = 1; i < points.size(); i++) {
            const godot::Vector2 &p = points[i];
            min.x = godot::Math::min(min.x, p.x);
            min.y = godot::Math::min(min.y, p.y);
            max.x = godot::Math::max(max.x, p.x);
            max.y = godot::Math::max(max.y, p.y);
        }
        payload.rect = {min, max - min};
        payload.points = godot::PackedVector2Array();
        payload.points.resize(points.size());
        payload.color = color;
        payload.rotation = godot::Math::deg_to_rad(rotation);
        payload.pivot = payload.rect.size*0.5f + pivot;
        for (int i = 0; i < points.size(); i++) {
            payload.points[i] = points[i] - payload.rect.position - payload.pivot;
        }
        payload.stroke = stroke;
        payload.stroke_points = godot::PackedVector2Array();
        payload.stroke_color = stroke_color;
        payload.stroke_points.resize(payload.points.size() + 1);
        if (payload.stroke > 0.0f) {
            float area = 0.0f;
            for (int i = 0; i < payload.points.size(); i++) {
                const godot::Vector2 &a = payload.points[i];
                const godot::Vector2 &b = payload.points[(i + 1)%payload.points.size()];
                area += (b.x - a.x)*(b.y + a.y);
            }
            bool clockwise = area > 0.0f;
            int count = payload.points.size();
            for (int i = 0; i < count; i++) {
                const godot::Vector2 &prev = payload.points[(i - 1 + count)%count];
                const godot::Vector2 &curr = payload.points[i];
                const godot::Vector2 &next = payload.points[(i + 1)%count];
                godot::Vector2 d1 = (curr - prev).normalized();
                godot::Vector2 d2 = (next - curr).normalized();
                godot::Vector2 n1 = {-d1.y, d1.x};
                godot::Vector2 n2 = {-d2.y, d2.x};
                if (clockwise) {
                    n1 = -n1;
                    n2 = -n2;
                }
                godot::Vector2 n = (n1 + n2).normalized();
                payload.stroke_points[i] = curr - n*payload.stroke*0.5;
            }
            payload.stroke_points[payload.points.size()] = payload.stroke_points[0];
        }
        push({Type::Polygon, payload});
    }

    void Canvas::draw_rectangle(
        godot::Vector2 position,
        godot::Vector2 size,
        const godot::Color& color,
        float stroke,
        const godot::Color& stroke_color,
        float rotation,
        godot::Vector2 pivot
    ) {
        Rectangle payload;
        payload.rect = {position, size};
        payload.color = color;
        payload.stroke = stroke;
        payload.stroke_color = stroke_color;
        payload.rotation = godot::Math::deg_to_rad(rotation);
        payload.pivot = pivot;
        push({Type::Rectangle, payload});
    }

    void Canvas::draw_circle(
        godot::Vector2 position,
        float radius,
        const godot::Color& color,
        float stroke,
        const godot::Color& stroke_color,
        float rotation,
        godot::Vector2 pivot
    ) {
        Circle payload;
        payload.position = position;
        payload.radius = radius;
        payload.color = color;
        payload.stroke = stroke;
        payload.stroke_color = stroke_color;
        payload.rotation = godot::Math::deg_to_rad(rotation);
        payload.pivot = pivot;
        push({Type::Circle, payload});
    }

    void Canvas::draw_image(
        godot::Vector2 position,
        godot::Vector2 size,
        Texture* texture,
        float rotation,
        godot::Vector2 pivot,
        const godot::Color& color
    ) {
        texture -> heartbeat();
        draw_image(position, size, texture -> get_texture(), rotation, pivot, color);
    }

    void Canvas::draw_image(
        godot::Vector2 position,
        godot::Vector2 size,
        RenderTarget* rt,
        float rotation,
        godot::Vector2 pivot,
        const godot::Color& color
    ) {
        if (!rt) return;
        draw_image(position, size, rt -> get_texture(), rotation, pivot, color);
    }

    void Canvas::draw_image(
        godot::Vector2 position,
        godot::Vector2 size,
        const std::string& path,
        float rotation,
        godot::Vector2 pivot,
        const godot::Color& color
    ) {
        try {
            auto texture = Texture::get_from_ref(path);
            if (!texture) texture = Texture::create_texture_2d(path, path);
            draw_image(position, size, texture, rotation, pivot, color);
        }
        catch(...) { std::rethrow_exception(std::current_exception()); }
    }

    void Canvas::draw_image(
        godot::Vector2 position,
        godot::Vector2 size,
        const godot::Ref<godot::Texture2D>& texture,
        float rotation,
        godot::Vector2 pivot,
        const godot::Color& color
    ) {
        if (!texture.is_valid()) return;
        Image payload;
        payload.texture = texture;
        payload.rect = {position, size};
        payload.rotation = godot::Math::deg_to_rad(rotation);
        payload.pivot = pivot;
        payload.color = color;
        push({Type::IMAGE, payload});
    }

    void Canvas::draw_text(
        const std::string& text,
        godot::Vector2 start_at,
        godot::Vector2 end_at,
        const godot::Ref<godot::Font>& font,
        int font_size,
        const godot::Color& color,
        std::pair<godot::HorizontalAlignment, godot::VerticalAlignment> alignment,
        bool clip,
        bool wordwrap,
        int stroke,
        const godot::Color& stroke_color,
        float rotation,
        godot::Vector2 pivot
    ) {
        if (!font.is_valid() || text.empty()) return;
        Text payload;
        payload.text = to_godot_string(text);
        payload.rect = {start_at, end_at - start_at};
        payload.font = font;
        payload.font_size = font_size;
        payload.font_ascent = payload.font -> get_ascent(payload.font_size);
        payload.color = color;
        payload.alignment = alignment;
        payload.clip = clip;
        payload.wordwrap = wordwrap;
        payload.stroke = stroke;
        payload.stroke_color = stroke_color;
        payload.rotation = godot::Math::deg_to_rad(rotation);
        payload.pivot = pivot;
        payload.rect.position.y += payload.font_ascent;
        payload.text_size = payload.font -> get_multiline_string_size(
            payload.text,
            payload.alignment.first,
            payload.wordwrap ? payload.rect.size.x : -1,
            payload.font_size
        );
        if (payload.alignment.second == godot::VERTICAL_ALIGNMENT_CENTER) payload.rect.position.y += (payload.rect.size.y - payload.text_size.y)*0.5f;
        else if (payload.alignment.second == godot::VERTICAL_ALIGNMENT_BOTTOM) payload.rect.position.y += payload.rect.size.y - payload.text_size.y;
        payload.rect.size.y = payload.wordwrap ? payload.text_size.y : payload.rect.size.y;
        push({Type::TEXT, payload});
    }
}
#endif