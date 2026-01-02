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
#include <Vital.extension/Engine/public/singleton.h>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot::Engine {
    enum class CanvasType {
        IMAGE,
        TEXT
    };

    struct CanvasCommand {
        CanvasType type;
        godot::Vector2 position;
        godot::Rect2 rect;
        godot::String text;
        godot::Ref<godot::Texture2D> texture;
        godot::Ref<godot::Font> font;
        float rotation = 0.0f; // radians
        godot::Vector2 pivot {0, 0};
        godot::Color color {1, 1, 1, 1};
        int font_size = 1;
    };

    class Canvas : public godot::Node2D {
        GDCLASS(Canvas, godot::Node2D)
        protected:
            static void _bind_methods() {}
        private:
            std::vector<CanvasCommand> queue;
        public:
            // Instantiators //
            Canvas() {};
            ~Canvas() override = default;
            void _ready() override;
            void _clean();
            void _process(double delta) override;
            void _draw() override;

            
			// Utils //
            void drawImage(
                const godot::Ref<godot::Texture2D>& texture,
                float x, float y,
                float w, float h,
                float rotation = 0.0f,
                float pivot_x = 0.0f,
                float pivot_y = 0.0f,
                const godot::Color& color = godot::Color(1, 1, 1, 1)
            );
            void drawText(const godot::String& text, float x, float y, const godot::Ref<godot::Font>& font, int font_size, const godot::Color& color = godot::Color(1, 1, 1, 1));
    };
}