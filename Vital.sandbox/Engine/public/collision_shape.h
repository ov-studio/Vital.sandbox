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
        friend class Network;
        private:
            // Instantiators //
            Collision_Shape();
            ~Collision_Shape() override;
            static void _bind_methods() {}
            void _notification(int what);

            #if defined(VSDK_Client)
            godot::Ref<godot::Shape3D> current_shape;
            godot::MeshInstance3D* debug_mesh = nullptr;
            // Every live client-side Collision_Shape registers itself here on
            // construction and removes itself on destruction. set_debug_all()
            // walks this directly — no dependency on the Lua vm_registry, so
            // it works identically for Lua-created AND network-replicated
            // shapes, and can't race against Lua Instance hydration.
            inline static std::unordered_set<Collision_Shape*> live_instances;
            inline static std::mutex live_instances_mutex;
            static godot::Ref<godot::ArrayMesh> build_wireframe_mesh(const godot::Ref<godot::Shape3D>& shape);
            #endif
        public:
            inline static std::function<void(Collision_Shape*)> on_spawned_callback;
            inline static std::function<void(Collision_Shape*)> on_destroyed_callback;

            
            // Managers //
            static Collision_Shape* create(godot::Node3D* owner);
            void destroy();

            // Applies a shape via Godot's native set_shape() AND (client-only)
            // keeps current_shape + the debug wireframe in sync. Every code path
            // that assigns a shape — Lua's set_shape_box/sphere/capsule/etc. and
            // Network::apply_shape's replicated assignment — should go through
            // this instead of calling set_shape() directly, so debug draw never
            // has to be wired up separately again.
            void assign_shape(godot::Ref<godot::Shape3D> shape);

            #if defined(VSDK_Client)
            void set_debug_visible(bool state);
            bool is_debug_visible() const { return debug_mesh && debug_mesh -> is_visible(); }
            void refresh_debug_mesh();

            inline static bool default_debug_enabled = false;

            // Client-only global toggle. Applies to every live shape right now
            // (regardless of creation path) and sets the default for shapes
            // created afterward.
            static void set_debug_all(bool state) {
                default_debug_enabled = state;
                std::lock_guard<std::mutex> lock(live_instances_mutex);
                for (auto* shape : live_instances) shape -> set_debug_visible(state);
            }
            #else
            void set_debug_visible(bool) {}
            bool is_debug_visible() const { return false; }
            static void set_debug_all(bool) {}
            #endif
    };
}
