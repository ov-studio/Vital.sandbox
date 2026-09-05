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

            // Fired from Physics_Body::setup_create()'s deferred enqueue once the parent
            // VehicleBody3D's net_id is registered and _spawn_entity has been sent.
            // The API layer sets this in Vehicle_Wheel::bind() to flush any pending
            // _spawn_wheel / _sync_wheel_config / _sync_wheel_transform RPCs that were
            // buffered because the vehicle body wasn't registered yet when the wheel
            // was created or configured in the same Lua tick as the vehicle body.
            // Receives the vehicle body Node3D so the callback can walk its children.
            inline static std::function<void(godot::Node3D*)> on_vehicle_ready_callback;

            // Index within the parent vehicle — used by RPCs to identify which wheel.
            int wheel_index = -1;

            
            // Managers //
            static Vehicle_Wheel* create(godot::Node3D* owner);
            void destroy();
    };
}
