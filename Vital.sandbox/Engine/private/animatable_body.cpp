/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: animatable_body.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Animatable Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/animatable_body.h>


/////////////////////////////////////
// Vital: Engine: Animatable_Body //
/////////////////////////////////////

namespace Vital::Engine {
    // Managers //
    Animatable_Body* Animatable_Body::create() {
        auto body = memnew(Animatable_Body);
        Engine::Core::get_singleton() -> add_child(body);
        return body;
    }

    void Animatable_Body::destroy() {
        queue_free();
    }
}
