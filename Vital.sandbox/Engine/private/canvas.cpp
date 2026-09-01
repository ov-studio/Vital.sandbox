/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: canvas.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Canvas Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/canvas.h>
#include <Vital.sandbox/Engine/public/font.h>
#include <Vital.sandbox/Engine/public/image.h>
#include <Vital.sandbox/Engine/public/svg.h>
#include <Vital.sandbox/Engine/public/rendertarget.h>
#include <Vital.sandbox/Engine/public/shader.h>
#include <Vital.sandbox/Manager/public/sandbox.h>


////////////////////////////
// Vital: Engine: Canvas //
////////////////////////////

namespace Vital::Engine {
    // Draw_Pool //
    godot::RID Canvas::Draw_Pool::next(godot::RID parent) {
        auto rs = godot::RenderingServer::get_singleton();
        idle_time = 0.0;
        if (used >= items.size()) {
            godot::RID item = rs -> canvas_item_create();
            rs -> canvas_item_set_parent(item, parent);
            items.push_back(item);
        }
        godot::RID item = items[used++];
        rs -> canvas_item_clear(item);
        rs -> canvas_item_set_visible(item, true);
        rs -> canvas_item_set_z_index(item, 0);
        rs -> canvas_item_set_material(item, godot::RID());
        return item;
    }

    void Canvas::Draw_Pool::end_frame(double delta) {
        auto rs = godot::RenderingServer::get_singleton();
        for (size_t i = used; i < items.size(); i++)
            rs -> canvas_item_set_visible(items[i], false);
        if (used == 0 && !items.empty()) {
            idle_time += delta;
            if (idle_time >= IDLE_SECONDS) free_all();
        }
        else idle_time = 0.0;
        used = 0;
    }

    void Canvas::Draw_Pool::free_all() {
        auto rs = godot::RenderingServer::get_singleton();
        for (auto& item : items) rs -> free_rid(item);
        items.clear();
        idle_time = 0.0;
    }


    // Hooks //
    void Canvas::_ready() {
        set_as_top_level(true);
        set_visible(true);
        set_process(true);
        set_z_index(godot::RenderingServer::CANVAS_ITEM_Z_MAX);
    }

    void Canvas::_process(double delta) {
        Engine::Texture::flush();
        Manager::Sandbox::get_singleton() -> draw(this);
        pool.end_frame(delta);
    }


    // Singleton //
    void Canvas::free_singleton() {
        if (!singleton) return;
        singleton -> teardown();
        singleton -> queue_free();
        singleton = nullptr;
    }

    void Canvas::teardown() {
        pool.free_all();
    }


    // Managers //
    void Canvas::init() {
        Engine::Core::get_singleton() -> add_child(singleton);
    }


    // Helpers //
    std::pair<Canvas::Draw_Pool*, godot::RID> Canvas::target() {
        auto rt = Engine::Rendertarget::get_active();
        if (rt) return {&rt -> get_pool(), rt -> get_canvas_item()};
        auto self = Canvas::get_singleton();
        return {&self -> pool, self -> get_canvas_item()};
    }

    void Canvas::notify_drawn() {
        auto rt = Engine::Rendertarget::get_active();
        if (rt) rt -> notify_drawn();
    }


    // Misc //
    godot::Vector3 Canvas::world_to_screen(godot::Vector3 position, float padding) {
        godot::Vector3 result = {-1, -1, -1};
        auto camera = Engine::Core::get_scene_root() -> get_camera_3d();
        if (camera) {
            auto camera_position = camera -> get_global_position();
            auto camera_forward = -camera -> get_global_transform().basis.get_column(2);
            auto screen_position = camera -> unproject_position(position);
            auto screen_size = Engine::Core::get_scene_root() -> get_visible_rect().size;
            if (
                (camera_forward.dot((position - camera_position).normalized()) > 0.0f) && 
                (screen_position.x >= -padding) && 
                (screen_position.y >= -padding) && 
                (screen_position.x <= screen_size.x + padding) && 
                (screen_position.y <= screen_size.y + padding)
            ) result = {screen_position.x, screen_position.y, camera_position.distance_to(position)};
        }
        return result;
    }

    godot::Vector3 Canvas::screen_to_world(godot::Vector2 position, float depth) {
        godot::Vector3 result = {-1, -1, -1};
        auto camera = Engine::Core::get_scene_root() -> get_camera_3d();
        if (camera) {
            auto origin = camera -> project_ray_origin(position);
            auto direction = camera -> project_ray_normal(position);
            result = origin + direction*depth;
        }
        return result;
    }

    void Canvas::draw_line(
        godot::PackedVector2Array points,
        float thickness,
        const godot::Color& color
    ) {
        if (points.size() < 2) return;
        auto [pool, parent] = target();
        auto rs = godot::RenderingServer::get_singleton();
        godot::RID item = pool -> next(parent);
        rs -> canvas_item_set_transform(item, godot::Transform2D());
        godot::PackedColorArray colors;
        colors.push_back(color);
        rs -> canvas_item_add_polyline(item, points, colors, thickness, true);
        Canvas::notify_drawn();
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
        godot::Vector2 min = points[0];
        godot::Vector2 max = points[0];
        for (int i = 1; i < points.size(); i++) {
            const godot::Vector2 &p = points[i];
            min.x = godot::Math::min(min.x, p.x);
            min.y = godot::Math::min(min.y, p.y);
            max.x = godot::Math::max(max.x, p.x);
            max.y = godot::Math::max(max.y, p.y);
        }
        godot::Rect2 rect = {min, max - min};
        godot::Vector2 local_pivot = rect.size*0.5f + pivot;
        godot::PackedVector2Array local_points;
        local_points.resize(points.size());
        for (int i = 0; i < points.size(); i++) {
            local_points[i] = points[i] - rect.position - local_pivot;
        }
        godot::PackedVector2Array stroke_points;
        stroke_points.resize(local_points.size() + 1);
        if (stroke > 0.0f) {
            float area = 0.0f;
            for (int i = 0; i < local_points.size(); i++) {
                const godot::Vector2 &a = local_points[i];
                const godot::Vector2 &b = local_points[(i + 1)%local_points.size()];
                area += (b.x - a.x)*(b.y + a.y);
            }
            bool clockwise = area > 0.0f;
            int count = local_points.size();
            for (int i = 0; i < count; i++) {
                const godot::Vector2 &prev = local_points[(i - 1 + count)%count];
                const godot::Vector2 &curr = local_points[i];
                const godot::Vector2 &next = local_points[(i + 1)%count];
                godot::Vector2 d1 = (curr - prev).normalized();
                godot::Vector2 d2 = (next - curr).normalized();
                godot::Vector2 n1 = {-d1.y, d1.x};
                godot::Vector2 n2 = {-d2.y, d2.x};
                if (clockwise) {
                    n1 = -n1;
                    n2 = -n2;
                }
                godot::Vector2 n = (n1 + n2).normalized();
                stroke_points[i] = curr - n*stroke*0.5;
            }
            stroke_points[local_points.size()] = stroke_points[0];
        }

        auto [pool, parent] = target();
        auto rs = godot::RenderingServer::get_singleton();
        godot::RID item = pool -> next(parent);
        rs -> canvas_item_set_transform(item, godot::Transform2D(godot::Math::deg_to_rad(rotation), rect.position + local_pivot));
        if (stroke > 0.0f) {
            godot::PackedColorArray stroke_colors;
            stroke_colors.push_back(stroke_color);
            rs -> canvas_item_add_polyline(item, stroke_points, stroke_colors, stroke, true);
        }
        godot::PackedColorArray colors;
        colors.push_back(color);
        rs -> canvas_item_add_polygon(item, local_points, colors, godot::PackedVector2Array(), godot::RID());
        Canvas::notify_drawn();
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
        auto piv = size*0.5f + pivot;
        auto [pool, parent] = target();
        auto rs = godot::RenderingServer::get_singleton();
        godot::RID item = pool -> next(parent);
        rs -> canvas_item_set_transform(item, godot::Transform2D(godot::Math::deg_to_rad(rotation), position + piv));
        if (stroke > 0.0f) {
            godot::Rect2 r(
                -piv - godot::Vector2(stroke*0.5f, stroke*0.5f),
                size + godot::Vector2(stroke, stroke)
            );
            if (stroke >= r.size.width || stroke >= r.size.height) rs -> canvas_item_add_rect(item, r.grow(0.5f*stroke), stroke_color, true);
            else {
                godot::PackedVector2Array pts;
                pts.resize(5);
                pts[0] = r.position;
                pts[1] = r.position + godot::Vector2(r.size.x, 0);
                pts[2] = r.position + r.size;
                pts[3] = r.position + godot::Vector2(0, r.size.y);
                pts[4] = r.position;
                godot::PackedColorArray cols;
                cols.push_back(stroke_color);
                rs -> canvas_item_add_polyline(item, pts, cols, stroke, true);
            }
        }
        rs -> canvas_item_add_rect(item, godot::Rect2(-piv, size), color, true);
        Canvas::notify_drawn();
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
        auto [pool, parent] = target();
        auto rs = godot::RenderingServer::get_singleton();
        godot::RID item = pool -> next(parent);
        rs -> canvas_item_set_transform(item, godot::Transform2D(godot::Math::deg_to_rad(rotation), position + pivot));
        if (stroke > 0.0f) {
            if (stroke >= 2.0f*godot::Math::max(radius, radius)) rs -> canvas_item_add_ellipse(item, -pivot, radius + 0.5f*stroke, radius + 0.5f*stroke, stroke_color, true);
            else {
                constexpr int SEGMENTS = 64;
                constexpr float TAU = 6.283185307179586f;
                godot::PackedVector2Array pts;
                pts.resize(SEGMENTS + 1);
                float step = TAU/SEGMENTS;
                for (int i = 0; i < SEGMENTS; i++) {
                    float angle = i*step;
                    pts[i] = godot::Vector2(godot::Math::cos(angle)*radius, godot::Math::sin(angle)*radius) - pivot;
                }
                pts[SEGMENTS] = pts[0];
                godot::PackedColorArray cols;
                cols.push_back(stroke_color);
                rs -> canvas_item_add_polyline(item, pts, cols, stroke, true);
            }
        }
        rs -> canvas_item_add_ellipse(item, -pivot, radius, radius, color, true);
        Canvas::notify_drawn();
    }

    void Canvas::draw_material(
        godot::Vector2 position,
        godot::Vector2 size,
        Texture* texture,
        float rotation,
        godot::Vector2 pivot,
        const godot::Color& color
    ) {
        texture -> heartbeat();
        draw_material(position, size, texture -> get_canvas_texture(), rotation, pivot, color);
    }

    void Canvas::draw_material(
        godot::Vector2 position,
        godot::Vector2 size,
        Rendertarget* rt,
        float rotation,
        godot::Vector2 pivot,
        const godot::Color& color
    ) {
        draw_material(position, size, rt -> get_texture(), rotation, pivot, color);
    }

    void Canvas::draw_material(
        godot::Vector2 position,
        godot::Vector2 size,
        Shader* shader,
        float rotation,
        godot::Vector2 pivot,
        const godot::Color& color
    ) {
        if (!shader) return;
        draw_material(position, size, shader -> get_material(), rotation, pivot, color);
    }

    void Canvas::draw_material(
        godot::Vector2 position,
        godot::Vector2 size,
        const std::string& path,
        float rotation,
        godot::Vector2 pivot,
        const godot::Color& color
    ) {
        try {
            auto texture = Engine::Texture::get_from_reference(path);
            if (!texture) texture = Engine::Image::create(path, path);
            draw_material(position, size, texture, rotation, pivot, color);
        }
        catch (...) { std::rethrow_exception(std::current_exception()); }
    }

    void Canvas::draw_material(
        godot::Vector2 position,
        godot::Vector2 size,
        const godot::Ref<godot::Texture2D>& texture,
        float rotation,
        godot::Vector2 pivot,
        const godot::Color& color
    ) {
        if (!texture.is_valid()) return;
        auto piv = size*0.5f + pivot;
        auto [pool, parent] = target();
        auto rs = godot::RenderingServer::get_singleton();
        godot::RID item = pool -> next(parent);
        rs -> canvas_item_set_transform(item, godot::Transform2D(godot::Math::deg_to_rad(rotation), position + piv));
        texture -> draw_rect(item, godot::Rect2(-piv, size), false, color);
        Canvas::notify_drawn();
    }

    void Canvas::draw_material(
        godot::Vector2 position,
        godot::Vector2 size,
        const godot::Ref<godot::ShaderMaterial>& material,
        float rotation,
        godot::Vector2 pivot,
        const godot::Color& color
    ) {
        if (!material.is_valid()) return;
        material -> set_shader_parameter("modulate", godot::Variant(color));
        auto piv = size*0.5f + pivot;
        auto [pool, parent] = target();
        auto rs = godot::RenderingServer::get_singleton();
        godot::RID item = pool -> next(parent);
        rs -> canvas_item_set_material(item, material -> get_rid());
        rs -> canvas_item_set_transform(item, godot::Transform2D(godot::Math::deg_to_rad(rotation), position + piv));
        rs -> canvas_item_add_rect(item, godot::Rect2(-piv, size), color, true);
        Canvas::notify_drawn();
    }

    void Canvas::draw_text(
        const std::string& text,
        godot::Vector2 start_at,
        godot::Vector2 end_at,
        Font* font,
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
        draw_text(text, start_at, end_at, font -> get_font(), font_size, color, alignment, clip, wordwrap, stroke, stroke_color, rotation, pivot);
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
        godot::String gd_text = Tool::to_godot_string(text);
        godot::Rect2 rect = {start_at, end_at - start_at};
        float font_ascent = font -> get_ascent(font_size);
        rect.position.y += font_ascent;
        godot::Vector2 text_size = font -> get_multiline_string_size(
            gd_text,
            godot::HORIZONTAL_ALIGNMENT_LEFT,
            wordwrap ? rect.size.x : -1,
            font_size
        );
        if (alignment.first == godot::HORIZONTAL_ALIGNMENT_CENTER) rect.position.x += (rect.size.x - text_size.x)*0.5f;
        else if (alignment.first == godot::HORIZONTAL_ALIGNMENT_RIGHT) rect.position.x += rect.size.x - text_size.x;
        if (alignment.second == godot::VERTICAL_ALIGNMENT_CENTER) rect.position.y += (rect.size.y - text_size.y)*0.5f;
        else if (alignment.second == godot::VERTICAL_ALIGNMENT_BOTTOM) rect.position.y += rect.size.y - text_size.y;
        rect.size.x = text_size.x;
        rect.size.y = wordwrap ? text_size.y : rect.size.y;
        godot::Vector2 piv = rect.size*0.5f + pivot;
        piv.y -= font_ascent;

        auto [pool, item_parent] = target();
        auto rs = godot::RenderingServer::get_singleton();
        godot::RID item = pool -> next(item_parent);
        rs -> canvas_item_set_transform(item, godot::Transform2D(godot::Math::deg_to_rad(rotation), rect.position + piv));
        if (stroke > 0) {
            font -> draw_multiline_string_outline(
                item, -piv, gd_text,
                godot::HORIZONTAL_ALIGNMENT_LEFT, rect.size.x, font_size,
                -1, stroke, stroke_color
            );
        }
        font -> draw_multiline_string(
            item, -piv, gd_text,
            godot::HORIZONTAL_ALIGNMENT_LEFT, rect.size.x, font_size,
            -1, color
        );
        Canvas::notify_drawn();
    }
}
#endif
