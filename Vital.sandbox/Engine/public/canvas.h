/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: canvas.h
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
#include <Vital.sandbox/Engine/public/core.h>


////////////////////////////
// Vital: Engine: Canvas //
////////////////////////////

// TODO: Use draw_image and accept shader as 'material' instead

namespace Vital::Engine {
    class Font;
    class Texture;
    class Rendertarget;
    class Shader;
    class Canvas : public godot::Node2D, public Tool::Base<Canvas> {
        GDCLASS(Canvas, godot::Node2D)
        friend class Tool::Base<Canvas>;
        public:
            static constexpr const char* Name = "Canvas.engine";

            enum class Type {
                Line,
                Polygon,
                Rectangle,
                Circle,
                IMAGE,
                SHADER,
                TEXT
            };
        
            struct Line {
                godot::PackedVector2Array points;
                float thickness;
                godot::Color color;
            };

            struct Polygon {
                godot::PackedVector2Array points;
                godot::Rect2 rect;
                godot::Color color;
                float stroke;
                godot::PackedVector2Array stroke_points;
                godot::Color stroke_color;
                float rotation;
                godot::Vector2 pivot;
            };

            struct Rectangle {
                godot::Rect2 rect;
                godot::Color color;
                float stroke;
                godot::Color stroke_color;
                float rotation;
                godot::Vector2 pivot;
            };

            struct Circle {
                godot::Vector2 position;
                float radius;
                godot::Color color;
                float stroke;
                godot::Color stroke_color;
                float rotation;
                godot::Vector2 pivot;
            };

            struct Image {
                godot::Ref<godot::Texture2D> texture;
                godot::Rect2 rect;
                float rotation;
                godot::Vector2 pivot;
                godot::Color color;
            };
        
            struct Text {
                godot::String text;
                godot::Rect2 rect;
                godot::Vector2 text_size;
                godot::Ref<godot::Font> font;
                int font_size;
                float font_height;
                float font_ascent;
                godot::Color color;
                std::pair<godot::HorizontalAlignment, godot::VerticalAlignment> alignment;
                bool clip;
                bool wordwrap;
                int stroke;
                godot::Color stroke_color;
                float rotation;
                godot::Vector2 pivot;
            };
        
            // Shader_Draw — draws a rect on the canvas using a ShaderMaterial.
            // The material is ref-counted; the Shader instance must outlive the draw call.
            struct Shader_Draw {
                godot::Ref<godot::ShaderMaterial> material;
                godot::Rect2 rect;
                float rotation;
                godot::Vector2 pivot;
                godot::Color color;  // modulate tint forwarded as shader param "modulate"
            };

            struct Command {
                Type type;
                std::variant<Line, Polygon, Rectangle, Circle, Image, Shader_Draw, Text> payload;
            };
        private:
            std::vector<Command> queue;


            // Instantiators //
            Canvas() = default;
            ~Canvas() override = default;
            static void _bind_methods() {}
        public:
            // Hooks //
            void _ready() override;
            void _process(double delta) override;
            void _draw() override;


            // Singleton //
            static void free_singleton();


            // Managers //
            void init();
            void push(Command command);
            static void execute(godot::Node2D* node, std::vector<Command>& queue);


            // Misc //
            godot::Vector3 world_to_screen(godot::Vector3 position, float padding = 0.0f);
            godot::Vector3 screen_to_world(godot::Vector2 position, float depth = 1.0f);

            void draw_line(
                godot::PackedVector2Array points,
                float thickness,
                const godot::Color& color = {1, 1, 1, 1}
            );
    
            void draw_polygon(
                godot::PackedVector2Array points,
                const godot::Color& color = {1, 1, 1, 1},
                float stroke = 0.0f,
                const godot::Color& stroke_color = {1, 1, 1, 1},
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f}
            );

            void draw_rectangle(
                godot::Vector2 position,
                godot::Vector2 size,
                const godot::Color& color = {1, 1, 1, 1},
                float stroke = 0.0f,
                const godot::Color& stroke_color = {1, 1, 1, 1},
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f}
            );

            void draw_circle(
                godot::Vector2 position,
                float radius,
                const godot::Color& color = {1, 1, 1, 1},
                float stroke = 0.0f,
                const godot::Color& stroke_color = {1, 1, 1, 1},
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f}
            );

            void draw_image(
                godot::Vector2 position,
                godot::Vector2 size,
                Texture* texture,
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f},
                const godot::Color& color = {1, 1, 1, 1}
            );

            void draw_image(
                godot::Vector2 position,
                godot::Vector2 size,
                Rendertarget* rt,
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f},
                const godot::Color& color = {1, 1, 1, 1}
            );

            void draw_image(
                godot::Vector2 position,
                godot::Vector2 size,
                const std::string& path,
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f},
                const godot::Color& color = {1, 1, 1, 1}
            );

            void draw_image(
                godot::Vector2 position,
                godot::Vector2 size,
                const godot::Ref<godot::Texture2D>& texture,
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f},
                const godot::Color& color = {1, 1, 1, 1}
            );

            // draw_shader — pushes a SHADER command into the canvas/RT queue.
            // The ShaderMaterial's canvas_item shader runs on a rect of (position, size).
            // `color` is forwarded as a "modulate" shader parameter so shaders can tint
            // or use it as an alpha; if the shader doesn't declare that uniform it is
            // silently ignored by Godot.
            void draw_shader(
                godot::Vector2 position,
                godot::Vector2 size,
                Shader* shader,
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f},
                const godot::Color& color = {1, 1, 1, 1}
            );

            void draw_shader(
                godot::Vector2 position,
                godot::Vector2 size,
                const godot::Ref<godot::ShaderMaterial>& material,
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f},
                const godot::Color& color = {1, 1, 1, 1}
            );
            
            void draw_text(
                const std::string& text,
                godot::Vector2 start_at,
                godot::Vector2 end_at,
                Font* font,
                int font_size,
                const godot::Color& color = {1, 1, 1, 1},
                std::pair<godot::HorizontalAlignment, godot::VerticalAlignment> alignment = {godot::HORIZONTAL_ALIGNMENT_LEFT, godot::VERTICAL_ALIGNMENT_CENTER},
                bool clip = false,
                bool wordwrap = false,
                int stroke = 0,
                const godot::Color& stroke_color = {1, 1, 1, 1},
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f}
            );

            void draw_text(
                const std::string& text,
                godot::Vector2 start_at,
                godot::Vector2 end_at,
                const godot::Ref<godot::Font>& font,
                int font_size,
                const godot::Color& color = {1, 1, 1, 1},
                std::pair<godot::HorizontalAlignment, godot::VerticalAlignment> alignment = {godot::HORIZONTAL_ALIGNMENT_LEFT, godot::VERTICAL_ALIGNMENT_CENTER},
                bool clip = false,
                bool wordwrap = false,
                int stroke = 0,
                const godot::Color& stroke_color = {1, 1, 1, 1},
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f}
            );
    };
}
#endif