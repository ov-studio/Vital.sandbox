/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: rendertarget.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rendertarget Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#include <Vital.extension/Engine/public/rendertarget.h>


/////////////////////////////////
// Vital: Godot: RenderTarget //
/////////////////////////////////

namespace Vital::Godot {
    // Instantiators //
    void RenderTarget::_draw() {
        Canvas::_execute(static_cast<godot::Node2D*>(this), queue);
        _clean();
    }


    // Getters //
    godot::Vector2i RenderTarget::getSize() {
        return viewport -> get_size();
    }

    godot::SubViewport* RenderTarget::getViewport() {
        return viewport;
    }

    godot::Ref<godot::ViewportTexture> RenderTarget::getTexture() {
        return viewport -> get_texture();
    }


    // APIs //
    RenderTarget* RenderTarget::create2D(int width, int height, bool transparent) {
        auto* rt = memnew(RenderTarget);
        rt -> viewport = memnew(godot::SubViewport);
        rt -> viewport -> set_size({width, height});
        rt -> viewport -> set_disable_3d(true);
        rt -> viewport -> set_transparent_background(transparent);
        rt -> viewport -> set_update_mode(godot::SubViewport::UPDATE_ALWAYS);
        rt -> viewport -> add_child(rt);
        return rt;
    }

    void RenderTarget::clear(bool clear, bool reload) {
        getViewport() -> set_clear_mode(clear ? godot::SubViewport::CLEAR_MODE_ONCE : godot::SubViewport::CLEAR_MODE_NEVER);
        if (clear) {
            _clean();
            queue_redraw();
        }
    }
}