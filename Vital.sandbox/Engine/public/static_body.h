/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: static_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Static Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/physics_body.h>


/////////////////////////////////
// Vital: Engine: Static_Body //
/////////////////////////////////

// TODO: Improve

namespace Vital::Manager { class Network; }

namespace Vital::Engine {
    class Static_Body : public PhysicsBodyBase<godot::StaticBody3D> {
        GDCLASS(Static_Body, godot::StaticBody3D)
        friend class Manager::Network;
        friend class Network;
        private:
            static void _bind_methods() {}
            Static_Body() = default;
            ~Static_Body() override = default;
        public:
            PhysicsSubType get_physics_sub_type() const override { return PhysicsSubType::Static; }

            void _ready() override { _ready_sync(pending_authority); }
            void _notification(int what) {
                if (what == NOTIFICATION_PREDELETE) _notify_predelete_sync();
            }
            // See Character_Body::_process for why this moved off _physics_process:
            // remote-side interpolation is a visual read-out of the snapshot buffer,
            // not a physics simulation step, so it should run once per rendered
            // frame via _process, matching Model, Animatable_Body and Vehicle_Body.
            void _process(double delta) override { on_sync_process(delta); }

            // Managers //
            static Static_Body* create(int authority_peer = 0);
            void destroy();

            #if !defined(VSDK_Client)
            using PhysicsBodyBase<godot::StaticBody3D>::set_syncer;
            #endif
    };
}
