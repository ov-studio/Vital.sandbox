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

#if defined(Vital_SDK_Client)
#pragma once
#include <Vital.extension/Engine/public/canvas.h>


//////////////////////////////////
// Vital: Engine: RenderTarget //
//////////////////////////////////

namespace Vital::Engine {
    class RenderTarget : public godot::Node2D {
        GDCLASS(RenderTarget, godot::Node2D)
        protected:
            godot::SubViewport* viewport = nullptr;
            inline static RenderTarget* target = nullptr;
            static void _bind_methods() {}
        private:
            std::vector<Canvas::Command> queue;
            bool instant = false;
        public:
            // Instantiators //
            RenderTarget() = default;
            ~RenderTarget() override;
            void _update();
            void _draw() override;


            // Managers //
            static RenderTarget* create(int width, int height, bool transparent);
            void push(Canvas::Command command);
            void clear(bool clear, bool instant);
        

            // Setters //
            static void set_target(RenderTarget* rt = nullptr, bool clear = false, bool instant = false);


            // Getters //
            godot::Vector2i get_size();
            godot::SubViewport* get_viewport();
            godot::Ref<godot::ViewportTexture> get_texture();
            static RenderTarget* get_target();
    };
}
#endif