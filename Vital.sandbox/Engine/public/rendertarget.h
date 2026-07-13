/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: rendertarget.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rendertarget Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/canvas.h>


//////////////////////////////////
// Vital: Engine: Rendertarget //
//////////////////////////////////

namespace Vital::Engine {
    class Rendertarget : public godot::Node2D {
        GDCLASS(Rendertarget, godot::Node2D)
        protected:
            godot::SubViewport* viewport = nullptr;
            inline static Rendertarget* active = nullptr;
        private:
            std::vector<Engine::Canvas::Command> queue;
            bool instant = false;


            // Instantiators //
            Rendertarget() = default;
            ~Rendertarget() override;
            static void _bind_methods() {}
        public:
            // Hooks //
            void _draw() override { draw(); };


            // Managers //
            static Rendertarget* create(godot::Vector2 size, bool transparent = false);
            void destroy();
            void push(Engine::Canvas::Command command);
            void clear(bool clear, bool instant);
            void update();
            void draw();


            // Checkers //
            bool is_active();


            // Getters //
            static Rendertarget* get_active();
            godot::Vector2 get_size();
            godot::SubViewport* get_viewport();
            godot::Ref<godot::ViewportTexture> get_texture();


            // Setters //
            static void set_active(Rendertarget* rt = nullptr, bool clear = false, bool instant = false);
    };
}
#endif