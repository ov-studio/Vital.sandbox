/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: collision_shape.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Collision Shape Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////////////
// Vital: Engine: Collision_Shape //
/////////////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    class Collision_Shape : public godot::CollisionShape3D {
        GDCLASS(Collision_Shape, godot::CollisionShape3D)
        private:
            // Instantiators //
            Collision_Shape() = default;
            ~Collision_Shape() override = default;
            static void _bind_methods() {}
            void _notification(int what) {
                if (what == NOTIFICATION_PREDELETE && on_destroyed_callback)
                    on_destroyed_callback(this);
            }
        public:
            // Single global callback fired on PREDELETE — lets the API layer
            // null and release any Lua Instance wrapping this node, same pattern
            // as PhysicsBodyBase::on_physics_body_destroyed_callback.
            inline static std::function<void(Collision_Shape*)> on_destroyed_callback;

            // Single global callback fired right after a Collision_Shape node is
            // created by Network::_sync_shape() on the client (remote-synced shape
            // on a networked physics body). Lets the API layer hydrate a Lua-facing
            // Instance for it, mirroring on_physics_body_spawned_callback — without
            // this, remote shapes exist physically but are invisible to Lua (no
            // entity:created, no debug wireframe, set_debug_all can't find them).
            inline static std::function<void(Collision_Shape*)> on_spawned_callback;

            // Managers //
            static Collision_Shape* create(godot::Node3D* owner);
            void destroy();
    };
}
