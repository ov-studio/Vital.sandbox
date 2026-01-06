/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: rendertarget.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rendertarget Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/canvas.h>


/////////////////////////////////
// Vital: Godot: RenderTarget //
/////////////////////////////////

namespace Vital::Godot {
    class RenderTarget : public godot::Node2D {
        GDCLASS(RenderTarget, godot::Node2D)
        private:
            std::vector<Canvas::Command> queue;
        protected:
            godot::SubViewport* viewport = nullptr;
            static inline RenderTarget* rendertarget = nullptr;
            static void _bind_methods() {}
        public:
            // Instantiators //
            RenderTarget() = default;
            ~RenderTarget() override = default;
            void _clean() { queue.clear(); }
            void _draw() override;


            // Getters //
            godot::Vector2i get_size();
            godot::SubViewport* get_viewport();
            godot::Ref<godot::ViewportTexture> get_texture();
            static RenderTarget* get_rendertarget();


            // APIs //
            static RenderTarget* create(int width, int height, bool transparent);
            static void set_rendertarget(RenderTarget* rt = nullptr, bool clear = false, bool reload = false);
            void push(Canvas::Command command);
            void clear(bool clear, bool reload);
    };
}