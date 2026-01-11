/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: canvas.h
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
#include <Vital.extension/Engine/public/index.h>


///////////////////////////
// Vital: Godot: Canvas //
///////////////////////////

namespace Vital::Godot {
    class Texture;
    class RenderTarget;
    class Canvas;
    class Canvas : public godot::Node2D {
        GDCLASS(Canvas, godot::Node2D)
        public:
            enum class Type {
                Line,
                Polygon,
                Rectangle,
                Circle,
                IMAGE,
                TEXT
            };
        
            struct Line {
                godot::PackedVector2Array points;
                float stroke;
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
        
            struct Command {
                Type type;
                std::variant<Line, Polygon, Rectangle, Circle, Image, Text> payload;
            };
        private:
            std::vector<Command> queue;
        protected:
            static inline Canvas* singleton = nullptr;
            static void _bind_methods() {}
        public:
            // Instantiators //
            Canvas() = default;
            ~Canvas() override = default;
            void _ready() override;
            void _process(double delta) override;
            void _clean();
            void _draw() override;


            // Utils //
            static Canvas* get_singleton();
            static void execute(godot::Node2D* node, std::vector<Command> queue);
            void push(Command command);


            // APIs //
            void draw_line(
                godot::PackedVector2Array points,
                float stroke,
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
                RenderTarget* rt,
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