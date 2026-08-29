/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: vehicle_body.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vehicle Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/vehicle_body.h>


//////////////////////////////////
// Vital: Engine: Vehicle_Body //
//////////////////////////////////

namespace Vital::Engine {
    // Managers //
    Vehicle_Body* Vehicle_Body::create() {
        auto body = memnew(Vehicle_Body);
        Engine::Core::get_singleton() -> add_child(body);
        return body;
    }

    void Vehicle_Body::destroy() {
        queue_free();
    }
}
