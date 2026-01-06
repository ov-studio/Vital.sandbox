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
    void RenderTarget::_draw() {
        Canvas::_execute(static_cast<godot::Node2D*>(this), queue);
        _clean();
    }
}