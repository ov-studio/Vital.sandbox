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
}

namespace Vital::Godot {
    class Canvas : public godot::Node2D {
        GDCLASS(Canvas, godot::Node2D)
        public:
            enum class Type {
                IMAGE,
                TEXT
            };
        
            struct ImageCommand {
                godot::Ref<godot::Texture2D> texture;
                godot::Rect2 rect;
                float rotation = 0.0f;
                godot::Vector2 pivot = {0, 0};
                godot::Color color = {1, 1, 1, 1};
            };
        
            struct TextCommand {
                godot::String text;
                godot::Vector2 text_size;
                int text_lines = -1;
                godot::Ref<godot::Font> font;
                int font_size = 16;
                float font_height;
                float font_ascent;
                godot::Vector2 position = {0, 0};
                godot::Rect2 rect;
                godot::Color color = {1, 1, 1, 1};
                godot::HorizontalAlignment align_x;
                godot::VerticalAlignment align_y;
                float rotation = 0.0f;
                godot::Vector2 pivot = {0, 0};
                bool clip = false;
                bool wordwrap = false;
            };
        
            struct Command {
                Type type;
                std::variant<ImageCommand, TextCommand> payload;
            };
        protected:
            static void _bind_methods() {}
        private:
            std::vector<Command> queue;
            std::unordered_map<std::string, godot::Ref<godot::Texture2D>> textures;
            Vital::Godot::RenderTarget* current_rt = nullptr;
            std::vector<Vital::Godot::RenderTarget*> owned_rts;
        public:
            // Instantiators //
            Canvas() = default;
            ~Canvas() override = default;
            void _ready() override;
            void _process(double delta) override;
            void clean() { queue.clear(); }
            void _draw() override;


            // Utils //
            godot::Ref<godot::Texture2D> fetch_texture(const std::string& path);
            Vital::Godot::RenderTarget* dx_create_rendertarget(int w, int h, bool transparent);
            void dx_set_rendertarget(Vital::Godot::RenderTarget* rt = nullptr, bool clear = false, bool reload = false);


            // APIs //
            void draw_image(
                float x, float y,
                float w, float h,
                const godot::Ref<godot::Texture2D>& texture,
                float rotation = 0.0f,
                float pivot_x = 0.0f,
                float pivot_y = 0.0f,
                const godot::Color& color = godot::Color(1, 1, 1, 1)
            );

            void draw_image(
                float x, float y,
                float w, float h,
                const std::string& path,
                float rotation = 0.0f,
                float pivot_x = 0.0f,
                float pivot_y = 0.0f,
                const godot::Color& color = godot::Color(1, 1, 1, 1)
            );

            void draw_image(
                float x, float y,
                float w, float h,
                Vital::Godot::RenderTarget* rt,
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

namespace Vital::Godot {
    class RenderTarget : public godot::Node2D {
        GDCLASS(RenderTarget, godot::Node2D)
        protected:
            static void _bind_methods() {}
        public:
            godot::SubViewport* viewport = nullptr;
            godot::Ref<godot::ViewportTexture> texture;
            godot::Vector2i size;
            RenderTarget() = default;
            ~RenderTarget() override = default;
            std::vector<Vital::Godot::Canvas::Command> queue;
            void clean() { queue.clear(); }
            void _draw() override;
    };
}