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

// TODO: Improve?
namespace Vital::Engine {
    class Vehicle_Wheel : public godot::VehicleWheel3D {
        GDCLASS(Vehicle_Wheel, godot::VehicleWheel3D)
        friend class Network;
        private:
            // Instantiators //
            Vehicle_Wheel() = default;
            ~Vehicle_Wheel() override = default;
            static void _bind_methods() {}
            void _notification(int what) {
                if (what == NOTIFICATION_PREDELETE && on_destroyed_callback)
                    on_destroyed_callback(this);
            }
        public:
            // Fired on PREDELETE — lets API layer null and release any dangling Lua Instance.
            inline static std::function<void(Vehicle_Wheel*)> on_destroyed_callback;

            // Index within the parent vehicle — used by RPCs to identify which wheel.
            int wheel_index = -1;

            
            // Managers //
            static Vehicle_Wheel* create(godot::Node3D* owner);
            void destroy();
    };
}
