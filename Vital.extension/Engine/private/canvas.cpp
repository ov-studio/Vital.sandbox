/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: canvas.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Canvas Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/canvas.h>
#include <Vital.extension/Sandbox/lua/public/index.h>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot::Engine {
    // Instantiators //
    Canvas::Canvas() {
        godot::UtilityFunctions::print("Initialized canvas ye");
    }
    
    void Canvas::_ready() {
    }
    
    void Canvas::_process(double delta) {
        set_position(get_viewport() -> get_visible_rect().position);
        queue_redraw();
    }
    
    void Canvas::_draw() {
        godot::UtilityFunctions::print("Drawing canvas");
        //Vital::Godot::Sandbox::Lua::Singleton::fetch() -> process(delta);
    }
}