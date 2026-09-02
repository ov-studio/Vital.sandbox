/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: character_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Character Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/physics_body.h>


////////////////////////////////////
// Vital: Engine: Character_Body //
////////////////////////////////////

// TODO: Improve

namespace Vital::Manager { class Network; }

namespace Vital::Engine {
    class Character_Body : public PhysicsBodyBase<godot::CharacterBody3D> {
        GDCLASS(Character_Body, godot::CharacterBody3D)
        friend class Manager::Network;
        friend class Network;
        private:
            // Instantiators //
            Character_Body() = default;
            ~Character_Body() override = default;
            static void _bind_methods() {}
        public:
            PhysicsSubType get_physics_sub_type() const override { return PhysicsSubType::Character; }

            void _ready() override { _ready_sync(pending_authority); }
            void _notification(int what) {
                if (what == NOTIFICATION_PREDELETE) _notify_predelete_sync();
            }
            // Interpolation is a purely visual read-out of the snapshot buffer (the
            // actual physics for this body happens on its authority peer only — see
            // PhysicsBodyBase::on_sync_process/apply_sync), so it belongs in _process
            // (once per rendered frame), not _physics_process (once per fixed physics
            // tick). Running it in _physics_process ties remote characters' visible
            // position to the physics tick rate instead of the display's frame rate,
            // which is exactly the stutter/lag described in _process() vs
            // _physics_process() write-ups: any frame between two physics ticks
            // re-renders the same stale transform. Model, Animatable_Body and
            // Vehicle_Body already do this correctly via _process.
            void _process(double delta) override { on_sync_process(delta); }

            // Managers //
            static Character_Body* create(int authority_peer = 0);
            void destroy();

            #if !defined(VSDK_Client)
            using PhysicsBodyBase<godot::CharacterBody3D>::set_syncer;
            #endif
    };
}
