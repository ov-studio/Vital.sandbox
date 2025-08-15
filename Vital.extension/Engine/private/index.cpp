/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: index.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Engine Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Engine/public/index.h>
#include <Sandbox/public/lua.h>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot::Engine {
    Singleton::Singleton() {
        Vital::Godot::Sandbox::Lua::fetch();
    }
    
    void Singleton::_bind_methods() {

    }
    
    void Singleton::_process(double delta) {
        Vital::Godot::Sandbox::Lua::fetch() -> process(delta);
    }
}