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

    enum class DrawType {
        IMAGE,
        TEXT
    };

    struct DrawCommand {
        DrawType type;

        // common
        godot::Color color{1, 1, 1, 1};

        // image
        godot::Ref<godot::Texture2D> texture;
        godot::Rect2 rect;

        // text
        godot::String text;
        godot::Vector2 position;
        godot::Ref<godot::Font> font;
        int font_size = 16;
    };

    class Canvas : public godot::Node2D {
        GDCLASS(Canvas, godot::Node2D)

    protected:
        static void _bind_methods() {}

    private:
        std::vector<DrawCommand> queue;

    public:
        Canvas();
        ~Canvas() override = default;
        void _ready() override;
        void _process(double delta) override;
        void _draw() override;
        inline void clear() { queue.clear(); }

        void dx_draw_image(
            const godot::Ref<godot::Texture2D>& texture,
            float x, float y, float w, float h,
            const godot::Color& color = godot::Color(1,1,1,1)
        );

        void dx_draw_text(
            const godot::String& text,
            float x, float y,
            const godot::Ref<godot::Font>& font,
            int font_size,
            const godot::Color& color = godot::Color(1,1,1,1)
        );
    };
}