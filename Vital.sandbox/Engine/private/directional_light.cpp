/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: directional_light.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Directional Light Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/directional_light.h>


///////////////////////////////////////
// Vital: Engine: Directional_Light //
///////////////////////////////////////

namespace Vital::Engine {
    // Managers //
    Directional_Light* Directional_Light::create() {
        auto light = memnew(Directional_Light);
        Engine::Core::get_singleton() -> add_child(light);
        return light;
    }

    void Directional_Light::destroy() {
        queue_free();
    }
}
#endif