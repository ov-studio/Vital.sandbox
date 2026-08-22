/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: vehicle_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vehicle Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////////
// Vital: Engine: Vehicle_Body //
/////////////////////////////////

namespace Vital::Engine {
    class Vehicle_Body : public godot::VehicleBody3D {
        GDCLASS(Vehicle_Body, godot::VehicleBody3D)
        private:
            // Instantiators //
            Vehicle_Body() = default;
            ~Vehicle_Body() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static Vehicle_Body* create();
            void destroy();
    };
}
#endif
