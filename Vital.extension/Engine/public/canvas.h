/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: canvas.h
     Author: vStudio
     Desc: Canvas Utilities (with MTA-like RenderTargets)
----------------------------------------------------------------*/

#pragma once

#include <Vital.extension/Engine/public/index.h>

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/font.hpp>

#include <variant>
#include <vector>
#include <unordered_map>

namespace Vital::Godot::Canvas {

    // ------------------------------------------------------------
    // Command Types
    // ------------------------------------------------------------
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

    // ------------------------------------------------------------
    // RenderTarget (dxRenderTarget)
    // ------------------------------------------------------------
    struct RenderTarget {
        godot::SubViewport *viewport = nullptr;
        godot::Node2D *canvas = nullptr; // <-- IMPORTANT
        godot::Ref<godot::ViewportTexture> texture;
        godot::Vector2i size;
    };
    

    class RTCanvas : public godot::Node2D {
        GDCLASS(RTCanvas, godot::Node2D)

    protected:
        static void _bind_methods() {}

    public:
        RTCanvas() = default;
        ~RTCanvas() override = default;
        std::vector<Command> queue;

        void clear() { queue.clear(); }

        void _draw() override;

    };


    // ------------------------------------------------------------
    // Canvas Singleton
    // ------------------------------------------------------------
    class Singleton : public godot::Node2D {
        GDCLASS(Singleton, godot::Node2D)

    protected:
        static void _bind_methods() {}

    private:
        std::vector<Command> queue;
        std::unordered_map<std::string, godot::Ref<godot::Texture2D>> textures;

        RenderTarget *current_rt = nullptr;
        std::vector<RenderTarget*> owned_rts;

    public:
        Singleton() = default;
        ~Singleton() override = default;

        void _ready() override;
        void _process(double delta) override;
        void _draw() override;

        // --------------------------------------------------------
        // RenderTarget API (MTA-like)
        // --------------------------------------------------------
        RenderTarget* dx_create_rendertarget(int w, int h, bool transparent);
        void dx_set_rendertarget(RenderTarget *rt = nullptr, bool clear = false);

        // --------------------------------------------------------
        // Utils
        // --------------------------------------------------------
        godot::Ref<godot::Texture2D> fetch_texture(const std::string &path);

        // --------------------------------------------------------
        // Draw APIs
        // --------------------------------------------------------
        void draw_image(
            const godot::Ref<godot::Texture2D> &texture,
            float x, float y,
            float w, float h,
            float rotation = 0.0f,
            float pivot_x = 0.0f,
            float pivot_y = 0.0f,
            const godot::Color &color = godot::Color(1,1,1,1)
        );

        void draw_image(
            const std::string &path,
            float x, float y,
            float w, float h,
            float rotation = 0.0f,
            float pivot_x = 0.0f,
            float pivot_y = 0.0f,
            const godot::Color &color = godot::Color(1,1,1,1)
        );

        void draw_image(
            RenderTarget *rt,
            float x, float y,
            float w, float h,
            float rotation = 0.0f,
            float pivot_x = 0.0f,
            float pivot_y = 0.0f,
            const godot::Color &color = godot::Color(1,1,1,1)
        );

        void draw_text(
            const godot::String &text,
            float left_x, float top_y,
            float right_x, float bottom_y,
            const godot::Ref<godot::Font> &font,
            int font_size,
            const godot::Color &color,
            godot::HorizontalAlignment align_x,
            godot::VerticalAlignment align_y,
            bool clip = false,
            bool wordwrap = false,
            float rotation = 0.0f,
            float pivot_x = 0.0f,
            float pivot_y = 0.0f
        );
    };
}
