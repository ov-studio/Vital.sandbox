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
            // Instantiators //
            Static_Body() = default;
            ~Static_Body() override = default;
            static void _bind_methods() {}
        public:
            PhysicsType get_physics_type() const override { return PhysicsType::Static; }

            // Node lifecycle (_ready/_notification/_process) lives in PhysicsBodyBase —
            // shared across every physics body subtype, see physics_body.h.

            // Managers //
            static Static_Body* create(int authority_peer = 0);
            void destroy();

            #if !defined(VSDK_Client)
            using PhysicsBodyBase<godot::StaticBody3D>::set_syncer;
            #endif
    };
}
