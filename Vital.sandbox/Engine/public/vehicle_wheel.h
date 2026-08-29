/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: vehicle_wheel.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vehicle Wheel Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>


///////////////////////////////////
// Vital: Engine: Vehicle_Wheel //
///////////////////////////////////

namespace Vital::Engine {
    class Vehicle_Wheel : public godot::VehicleWheel3D {
        GDCLASS(Vehicle_Wheel, godot::VehicleWheel3D)
        private:
            // Instantiators //
            Vehicle_Wheel() = default;
            ~Vehicle_Wheel() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static Vehicle_Wheel* create(godot::Node3D* owner);
            void destroy();
    };
}
