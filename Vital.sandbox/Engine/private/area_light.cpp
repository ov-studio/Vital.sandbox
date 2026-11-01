/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: area_light.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Area Light Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/area_light.h>


////////////////////////////////
// Vital: Engine: Area_Light //
////////////////////////////////

namespace Vital::Engine {
    // Managers //
    Area_Light* Area_Light::create() {
        auto light = memnew(Area_Light);
        Engine::Core::get_singleton() -> add_child(light);
        return light;
    }

    void Area_Light::destroy() {
        queue_free();
    }
}
#endif
