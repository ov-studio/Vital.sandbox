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

#pragma once
#include <Vital.extension/Engine/public/index.h>


///////////////////////////
// Vital: Godot: Canvas //
///////////////////////////

namespace Vital::Godot {
    class RenderTarget;
    class Canvas;
    class Canvas : public godot::Node2D {
        GDCLASS(Canvas, godot::Node2D)
        public:
            enum class Type {
                Rectangle,
                IMAGE,
                TEXT
            };
        
            struct RectangleCommand {
                godot::Rect2 rect;
                godot::Color color;
                bool filled;
                float stroke;
                float rotation;
                godot::Vector2 pivot;
            };

            struct ImageCommand {
                godot::Ref<godot::Texture2D> texture;
                godot::Rect2 rect;
                float rotation;
                godot::Vector2 pivot;
                godot::Color color;
            };
        
            struct TextCommand {
                godot::String text;
                godot::Rect2 rect;
                godot::Vector2 text_size;
                int text_lines;
                godot::Ref<godot::Font> font;
                int font_size;
                float font_height;
                float font_ascent;
                godot::Color color;
                godot::HorizontalAlignment align_x;
                godot::VerticalAlignment align_y;
                float rotation;
                godot::Vector2 pivot;
                bool clip;
                bool wordwrap;
            };
        
            struct Command {
                Type type;
                std::variant<RectangleCommand, ImageCommand, TextCommand> payload;
            };
        private:
            std::vector<Command> queue;
            std::unordered_map<std::string, godot::Ref<godot::Texture2D>> textures;
        protected:
            static inline Canvas* singleton = nullptr;
            static void _bind_methods() {}
        public:
            // Instantiators //
            Canvas() = default;
            ~Canvas() override = default;
            void _ready() override;
            void _process(double delta) override;
            void _clean() { queue.clear(); }
            void _draw() override;


            // Utils //
            static Canvas* get_singleton();
            godot::Ref<godot::Texture2D> get_texture(const std::string& path);
            static void execute(godot::Node2D* node, std::vector<Command> queue);
            void push(Command command);


            // APIs //
            void draw_rectangle(
                float x, float y,
                float width, float height,
                bool filled = true,
                float stroke = 0.0f,
                float rotation = 0.0f,
                float pivot_x = 0.0f,
                float pivot_y = 0.0f,
                const godot::Color& color = godot::Color(1, 1, 1, 1)
            );

            void draw_image(
                float x, float y,
                float width, float height,
                const godot::Ref<godot::Texture2D>& texture,
                float rotation = 0.0f,
                float pivot_x = 0.0f,
                float pivot_y = 0.0f,
                const godot::Color& color = godot::Color(1, 1, 1, 1)
            );

            void draw_image(
                float x, float y,
                float width, float height,
                const std::string& path,
                float rotation = 0.0f,
                float pivot_x = 0.0f,
                float pivot_y = 0.0f,
                const godot::Color& color = godot::Color(1, 1, 1, 1)
            );

            void draw_image(
                float x, float y,
                float width, float height,
                RenderTarget* rt,
                float rotation = 0.0f,
                float pivot_x = 0.0f,
                float pivot_y = 0.0f,
                const godot::Color& color = godot::Color(1, 1, 1, 1)
            );

            void draw_text(
                const godot::String& text,
                float left_x, float top_y,
                float right_x, float bottom_y,
                const godot::Ref<godot::Font> &font,
                int font_size,
                const godot::Color& color = godot::Color(1, 1, 1, 1),
                godot::HorizontalAlignment align_x = godot::HORIZONTAL_ALIGNMENT_LEFT,
                godot::VerticalAlignment align_y = godot::VERTICAL_ALIGNMENT_CENTER,
                bool clip = false,
                bool wordwrap = false,
                float rotation = 0.0f,
                float pivot_x = 0.0f,
                float pivot_y = 0.0f
            );
    };
}