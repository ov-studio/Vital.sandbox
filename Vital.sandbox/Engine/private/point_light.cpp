/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: point_light.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Point Light Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/point_light.h>


/////////////////////////////////
// Vital: Engine: Point_Light //
/////////////////////////////////

namespace Vital::Engine {
    // Managers //
    Point_Light* Point_Light::create() {
        auto light = memnew(Point_Light);
        Engine::Core::get_singleton() -> add_child(light);
        return light;
    }

    void Point_Light::destroy() {
        queue_free();
    }
}
#endif