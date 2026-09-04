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

            static godot::Ref<godot::ArrayMesh> build_wireframe_mesh(const godot::Ref<godot::Shape3D>& shape, const godot::Color& color);
            #endif
        public:
            inline static std::function<void(Collision_Shape*)> on_spawned_callback;
            inline static std::function<void(Collision_Shape*)> on_destroyed_callback;

            // Managers //
            static Collision_Shape* create(godot::Node3D* owner);
            void destroy();

            // True if this shape's owning body is a synced (server-authoritative,
            // network-replicated) entity — i.e. it arrived via Network::apply_shape
            // rather than being made purely client-side by Lua. Used to color the
            // debug wireframe differently so it's obvious at a glance which shapes
            // are server-driven vs. local-only.
            // NOTE: implemented in collision_shape.cpp, not inline here — this
            // header is included from several TUs with different include orders
            // (Vital/engine.h, Manager/private/network.cpp, Sandbox/runtime/apis.h,
            // ...) and inlining a dynamic_cast<ISyncable*> here meant every one of
            // them had to already have ISyncable fully defined at this exact point,
            // which wasn't reliably true and caused a C2061 on some TUs. Keeping
            // ISyncable's #include local to the .cpp avoids that entirely.
            bool is_replicated() const;

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

            // Debug wireframe colors — green for shapes made locally on this
            // client, orange for shapes that arrived via network replication
            // (server-authoritative bodies). Tweak these if you want different
            // colors.
            inline static godot::Color local_debug_color      = godot::Color(0, 1, 0);
            inline static godot::Color replicated_debug_color = godot::Color(1, 0.55f, 0);

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
