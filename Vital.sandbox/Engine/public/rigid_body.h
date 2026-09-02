/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: rigid_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rigid Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/physics_body.h>


////////////////////////////////
// Vital: Engine: Rigid_Body //
////////////////////////////////

// TODO: Improve

namespace Vital::Manager { class Network; }

namespace Vital::Engine {
    class Rigid_Body : public PhysicsBodyBase<godot::RigidBody3D> {
        GDCLASS(Rigid_Body, godot::RigidBody3D)
        friend class Manager::Network;
        friend class Network;
        private:
            // Instantiators //
            Rigid_Body() = default;
            ~Rigid_Body() override = default;
            static void _bind_methods() {}
        public:
            PhysicsType get_physics_type() const override { return PhysicsType::Rigid; }

            void _ready() override { _ready_sync(pending_authority); }
            void _process(double delta) override { on_sync_process(delta); }

            
            // Managers //
            static Rigid_Body* create(int authority_peer = 0);
            void destroy();
    };
}
