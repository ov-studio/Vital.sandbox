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
#include <Vital.sandbox/Engine/public/physics_body.h>


//////////////////////////////////
// Vital: Engine: Vehicle_Body //
//////////////////////////////////

// TODO: Improve

namespace Vital::Manager { class Network; }

namespace Vital::Engine {
    class Vehicle_Body : public PhysicsBodyBase<godot::VehicleBody3D> {
        GDCLASS(Vehicle_Body, godot::VehicleBody3D)
        friend class Manager::Network;
        private:
            int pending_authority = 1;
            static void _bind_methods() {}
            Vehicle_Body() = default;
            ~Vehicle_Body() override = default;
        public:
            PhysicsSubType get_physics_sub_type() const override { return PhysicsSubType::Vehicle; }

            void _ready() override { _ready_sync(pending_authority); }
            void _notification(int what) {
                if (what == NOTIFICATION_PREDELETE) _notify_predelete_sync();
            }
            void _process(double delta) override { on_sync_process(delta); }

            // Managers //
            static Vehicle_Body* create(int authority_peer = 0);
            void destroy();

            #if !defined(VSDK_Client)
            using PhysicsBodyBase<godot::VehicleBody3D>::set_syncer;
            #endif
    };
}
