/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: singleton.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Singleton Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Engine/public/singleton.h>
#include <Sandbox/public/lua.h>


//////////////////////////////
// Vital: Godot: Singleton //
//////////////////////////////

namespace Vital::Godot {
    Singleton::Singleton() {
        ExampleLUA::fetch();
    }
    
    void Singleton::_bind_methods() {
    
    }
    
    void Singleton::_process(double delta) {
    
    }
}