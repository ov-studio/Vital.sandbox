/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: rendertarget.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rendertarget Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.extension/Engine/public/canvas.h>


//////////////////////////////////
// Vital: Engine: Rendertarget //
//////////////////////////////////

namespace Vital::Engine {
    class Rendertarget : public godot::Node2D {
        GDCLASS(Rendertarget, godot::Node2D)
        protected:
            godot::SubViewport* viewport = nullptr;
            inline static Rendertarget* active = nullptr;
            static void _bind_methods() {}
        private:
            std::vector<Canvas::Command> queue;
            bool instant = false;
        public:
            // Instantiators //
            Rendertarget() = default;
            ~Rendertarget() override;
            void _update();
            void _draw() override;


            // Managers //
            static Rendertarget* create(int width, int height, bool transparent = false);
            void destroy();
            void push(Canvas::Command command);
            void clear(bool clear, bool instant);
        

            // Checkers //
            bool is_active();


            // Setters //
            static void set_active(Rendertarget* rt = nullptr, bool clear = false, bool instant = false);


            // Getters //
            godot::Vector2i get_size();
            godot::SubViewport* get_viewport();
            godot::Ref<godot::ViewportTexture> get_texture();
            static Rendertarget* get_active();
    };
}
#endif