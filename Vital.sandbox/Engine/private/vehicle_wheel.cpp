/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: vehicle_wheel.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vehicle Wheel Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/vehicle_wheel.h>


//////////////////////////////////
// Vital: Engine: Vehicle_Wheel //
//////////////////////////////////

namespace Vital::Engine {
    // Managers //
    Vehicle_Wheel* Vehicle_Wheel::create(godot::Node3D* owner) {
        auto wheel = memnew(Vehicle_Wheel);
        if (owner) owner -> add_child(wheel);
        else Engine::Core::get_singleton() -> add_child(wheel);
        return wheel;
    }

    void Vehicle_Wheel::destroy() {
        queue_free();
    }
}
#endif
