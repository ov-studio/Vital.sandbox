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

            struct Draw_Pool {
                std::vector<godot::RID> items;
                size_t used = 0;
                double idle_time = 0.0;
                static constexpr double IDLE_SECONDS = 5.0;

                godot::RID next(godot::RID parent);
                void end_frame(double delta);
                void free_all();
            };
        private:
            Draw_Pool pool;


            // Instantiators //
            Canvas() = default;
            ~Canvas() override = default;
            static void _bind_methods() {}


            // Helpers //
            static std::pair<Draw_Pool*, godot::RID> target();
            static void notify_drawn();
        public:
            // Hooks //
            void _ready() override;
            void _process(double delta) override;


            // Singleton //
            static void free_singleton();
            void teardown();


            // Managers //
            void init();


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

            void draw_material(
                godot::Vector2 position,
                godot::Vector2 size,
                Texture* texture,
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f},
                const godot::Color& color = {1, 1, 1, 1}
            );

            void draw_material(
                godot::Vector2 position,
                godot::Vector2 size,
                Rendertarget* rt,
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f},
                const godot::Color& color = {1, 1, 1, 1}
            );

            void draw_material(
                godot::Vector2 position,
                godot::Vector2 size,
                const std::string& path,
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f},
                const godot::Color& color = {1, 1, 1, 1}
            );

            void draw_material(
                godot::Vector2 position,
                godot::Vector2 size,
                const godot::Ref<godot::Texture2D>& texture,
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f},
                const godot::Color& color = {1, 1, 1, 1}
            );

            void draw_material(
                godot::Vector2 position,
                godot::Vector2 size,
                Shader* shader,
                float rotation = 0.0f,
                godot::Vector2 pivot = {0.0f, 0.0f},
                const godot::Color& color = {1, 1, 1, 1}
            );

            void draw_material(
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