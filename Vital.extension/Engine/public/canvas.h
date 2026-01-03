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

namespace Vital::Godot::Canvas {
    enum class Type {
        IMAGE,
        TEXT
    };

    struct Command {
        Type type;
        godot::Vector2 position;
        godot::Rect2 rect;
        godot::String text;
        godot::Ref<godot::Texture2D> texture;
        godot::Ref<godot::Font> font;
        float rotation = 0.0f;
        godot::Vector2 pivot {0, 0};
        godot::Color color {1, 1, 1, 1};
        int font_size = 1;
    };

    class Singleton : public godot::Node2D {
        GDCLASS(Singleton, godot::Node2D)
        protected:
            static void _bind_methods() {}
        private:
            std::vector<Command> queue;
            std::unordered_map<std::string, godot::Ref<godot::Texture2D>> cache;
        public:
            // Instantiators //
            Singleton() {};
            ~Singleton() override = default;
            void _ready() override;
            void _clean();
            void _process(double delta) override;
            void _draw() override;

            
            // Utils //
            godot::Ref<godot::Texture2D> get_texture_from_path(const std::string& path);

            void drawImage(
                const std::string& path,
                float x, float y,
                float w, float h,
                float rotation = 0.0f,
                float pivot_x = 0.0f,
                float pivot_y = 0.0f,
                const godot::Color& color = godot::Color(1, 1, 1, 1)
            );
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