/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: spot_light.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Spot Light Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/spot_light.h>


////////////////////////////////
// Vital: Engine: Spot_Light //
////////////////////////////////

namespace Vital::Engine {
    // Managers //
    Spot_Light* Spot_Light::create() {
        auto light = memnew(Spot_Light);
        Engine::Core::get_singleton() -> add_child(light);
        return light;
    }

    void Spot_Light::destroy() {
        queue_free();
    }
}
#endif