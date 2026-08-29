/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: area.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Area Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/area.h>


//////////////////////////
// Vital: Engine: Area //
//////////////////////////

namespace Vital::Engine {
    // Managers //
    Area* Area::create() {
        auto body = memnew(Area);
        Engine::Core::get_singleton() -> add_child(body);
        return body;
    }

    void Area::destroy() {
        queue_free();
    }
}
