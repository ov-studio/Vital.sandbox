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

#include <Vital.extension/Engine/public/canvas.h>
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
            //if (!godot::Engine::get_singleton() -> is_editor_hint()) {
                auto* root = Core::get_singleton() -> get_tree() -> get_root();
                singleton = memnew(Canvas);
                root -> call_deferred("add_child", singleton);
            //}
        }
        return singleton;
    }

    godot::Ref<godot::Texture2D> Canvas::get_texture(const std::string& path) {
        auto it = textures.find(path);
        if (it != textures.end()) return it -> second;
        auto texture = godot::ResourceLoader::get_singleton() -> load(path.c_str(), "Texture2D");
        if (texture.is_valid()) textures[path] = texture;
        return texture;
    }

    void Canvas::execute(godot::Node2D* node, std::vector<Command> queue) {
        for (const auto &command : queue) {
            switch (command.type) {
                case Type::Rectangle: {
                    const auto &payload = std::get<RectangleCommand>(command.payload);
                    auto pivot = payload.rect.size*0.5f + payload.pivot;
                    node -> draw_set_transform(payload.rect.position + pivot, payload.rotation, {1, 1});
                    node -> draw_rect(
                        godot::Rect2(-pivot, payload.rect.size),
                        payload.color,
                        payload.filled,
                        payload.filled ? -1 : payload.stroke,
                        true
                    );
                    break;
                }
                case Type::Circle: {
                    const auto &payload = std::get<CircleCommand>(command.payload);
                    auto pivot = payload.pivot;
                    node -> draw_set_transform(payload.rect.position + pivot, payload.rotation, {1, 1});
                    node -> draw_circle(
                        -pivot,
                        payload.rect.size.x,
                        payload.color,
                        payload.filled,
                        payload.filled ? -1 : payload.stroke,
                        true
                    );
                    break;
                }
                case Type::Line: {
                    const auto &payload = std::get<LineCommand>(command.payload);
                    node -> draw_set_transform({0, 0}, 0, {1, 1});
                    node -> draw_polyline(
                        payload.points,
                        payload.color,
                        payload.stroke,
                        true
                    );
                    break;
                }
                case Type::IMAGE: {
                    const auto &payload = std::get<ImageCommand>(command.payload);
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
                    const auto &payload = std::get<TextCommand>(command.payload);
                    auto pivot = payload.rect.size*0.5f + payload.pivot;
                    pivot.y -= payload.font_ascent;
                    node -> draw_set_transform(payload.rect.position + pivot, payload.rotation, {1, 1});
                    if (payload.stroke > 0) {
                        node -> draw_multiline_string_outline(
                            payload.font,
                            -pivot,
                            payload.text,
                            payload.align_x,
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
                        payload.align_x,
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
    void Canvas::draw_rectangle(
        float x, float y,
        float width, float height,
        bool filled,
        float stroke,
        float rotation,
        float pivot_x,
        float pivot_y,
        const godot::Color& color
    ) {
        RectangleCommand payload;
        payload.rect = {x, y, width, height};
        payload.filled = filled;
        payload.stroke = stroke;
        payload.rotation = godot::Math::deg_to_rad(rotation);
        payload.pivot = {pivot_x, pivot_y};
        payload.color = color;
        push({Type::Rectangle, payload});
    }

    void Canvas::draw_circle(
        float x, float y,
        float radius,
        bool filled,
        float stroke,
        float rotation,
        float pivot_x,
        float pivot_y,
        const godot::Color& color
    ) {
        CircleCommand payload;
        payload.rect = {x, y, radius, radius};
        payload.filled = filled;
        payload.stroke = stroke;
        payload.rotation = godot::Math::deg_to_rad(rotation);
        payload.pivot = {pivot_x, pivot_y};
        payload.color = color;
        push({Type::Circle, payload});
    }

    void Canvas::draw_line(
        godot::PackedVector2Array points,
        float stroke,
        const godot::Color& color
    ) {
        LineCommand payload;
        payload.points = points;
        payload.stroke = stroke;
        payload.color = color;
        push({Type::Line, payload});
    }

    void Canvas::draw_image(
        float x, float y,
        float width, float height,
        const godot::Ref<godot::Texture2D>& texture,
        float rotation,
        float pivot_x, float pivot_y,
        const godot::Color& color
    ) {
        if (!texture.is_valid()) return;
        ImageCommand payload;
        payload.texture = texture;
        payload.rect = {x, y, width, height};
        payload.rotation = godot::Math::deg_to_rad(rotation);
        payload.pivot = {pivot_x, pivot_y};
        payload.color = color;
        push({Type::IMAGE, payload});

    }

    void Canvas::draw_image(
        float x, float y,
        float width, float height,
        const std::string& path,
        float rotation,
        float pivot_x, float pivot_y,
        const godot::Color& color
    ) {
        draw_image(x, y, width, height, get_texture(path), rotation, pivot_x, pivot_y, color);
    }

    void Canvas::draw_image(
        float x, float y,
        float width, float height,
        RenderTarget* rt,
        float rotation,
        float pivot_x, float pivot_y,
        const godot::Color& color
    ) {
        if (!rt) return;
        draw_image(x, y, width, height, rt -> get_texture(), rotation, pivot_x, pivot_y, color);
    }

    void Canvas::draw_text(
        const godot::String& text,
        float left_x, float top_y,
        float right_x, float bottom_y,
        const godot::Ref<godot::Font>& font,
        int font_size,
        const godot::Color& color,
        godot::HorizontalAlignment alignment_x, godot::VerticalAlignment alignment_y,
        bool clip,
        bool wordwrap,
        int stroke,
        const godot::Color& stroke_color,
        float rotation,
        float pivot_x, float pivot_y
    ) {
        if (!font.is_valid() || text.is_empty()) return;
        TextCommand payload;
        payload.text = text;
        payload.rect = {left_x, top_y, right_x - left_x, bottom_y - top_y};
        payload.font = font;
        payload.font_size = font_size;
        payload.font_ascent = payload.font -> get_ascent(payload.font_size);
        payload.color = color;
        payload.align_x = alignment_x;
        payload.align_y = alignment_y;
        payload.clip = clip;
        payload.wordwrap = wordwrap;
        payload.stroke = stroke;
        payload.stroke_color = stroke_color;
        payload.rotation = godot::Math::deg_to_rad(rotation);
        payload.pivot = {pivot_x, pivot_y};
        payload.rect.position.y += payload.font_ascent;
        payload.text_size = payload.font -> get_multiline_string_size(
            payload.text,
            payload.align_x,
            payload.wordwrap ? payload.rect.size.x : -1,
            payload.font_size
        );
        if (payload.align_y == godot::VERTICAL_ALIGNMENT_CENTER) payload.rect.position.y += (payload.rect.size.y - payload.text_size.y)*0.5f;
        else if (payload.align_y == godot::VERTICAL_ALIGNMENT_BOTTOM) payload.rect.position.y += payload.rect.size.y - payload.text_size.y;
        payload.rect.size.y = payload.wordwrap ? payload.text_size.y : payload.rect.size.y;
        push({Type::TEXT, payload});
    }
}
