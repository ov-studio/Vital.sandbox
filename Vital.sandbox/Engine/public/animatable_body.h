/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: animatable_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Animatable Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/physics_body.h>


/////////////////////////////////////
// Vital: Engine: Animatable_Body //
/////////////////////////////////////

// TODO: Improve

namespace Vital::Manager { class Network; }

namespace Vital::Engine {
    class Animatable_Body : public PhysicsBodyBase<godot::AnimatableBody3D> {
        GDCLASS(Animatable_Body, godot::AnimatableBody3D)
        friend class Manager::Network;
        friend class Network;
        private:
            // Instantiators //
            Animatable_Body() = default;
            ~Animatable_Body() override = default;
            static void _bind_methods() {}
        public:
            PhysicsType get_physics_type() const override { return PhysicsType::Animatable; }

            // Node lifecycle (_ready/_notification/_process) lives in PhysicsBodyBase —
            // shared across every physics body subtype, see physics_body.h.

            // Managers //
            static Animatable_Body* create(int authority_peer = 0);
            void destroy();
            // set_syncer() is already public on PhysicsBodyBase — no need to
            // re-expose it here.
    };
}
