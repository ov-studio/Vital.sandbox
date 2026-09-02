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

            // _notification is shared via PhysicsBodyBase (see physics_body.h).
            // _ready/_process must be redeclared per leaf class — godot-cpp's
            // GDCLASS registration needs them on this exact class (see the NOTE
            // in physics_body.h) — but just delegate to the shared helpers so
            // there's no duplicated logic.
            void _ready() override { _ready_sync(pending_authority); }
            void _process(double delta) override { on_sync_process(delta); }

            
            // Managers //
            static Animatable_Body* create(int authority_peer = 0);
            void destroy();
    };
}
