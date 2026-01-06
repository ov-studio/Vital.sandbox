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
        protected:
            static void _bind_methods() {}
        public:
            godot::SubViewport* viewport = nullptr;
            godot::Ref<godot::ViewportTexture> texture;
            godot::Vector2i size;
            RenderTarget() = default;
            ~RenderTarget() override = default;
            std::vector<Canvas::Command> queue;
            void _clean() { queue.clear(); }
            void _draw() override;
    };
}