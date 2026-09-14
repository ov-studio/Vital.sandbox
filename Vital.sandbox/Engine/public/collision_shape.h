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
#include <Vital.sandbox/Engine/public/syncable.h>


/////////////////////////////////////
// Vital: Engine: Collision_Shape //
/////////////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    class Collision_Shape : public godot::CollisionShape3D {
        GDCLASS(Collision_Shape, godot::CollisionShape3D)
        friend class Network;
        private:
            #if defined(VSDK_Client)
            godot::Ref<godot::Shape3D> current_shape;
            godot::MeshInstance3D* debug_mesh = nullptr;
            // Every live client-side Collision_Shape registers itself here on
            // construction and removes itself on destruction. set_debug_all()
            // walks this directly — no dependency on the Lua vm_registry.
            inline static std::unordered_set<Collision_Shape*> live_instances;
            inline static std::mutex live_instances_mutex;
            #endif


            // Instantiators //
            Collision_Shape();
            ~Collision_Shape() override;
            static void _bind_methods() {}


            // Helpers //
            #if defined(VSDK_Client)
            static godot::Ref<godot::ArrayMesh> build_wireframe_mesh(const godot::Ref<godot::Shape3D>& shape, const godot::Color& color);
            #endif
        public:
            #if defined(VSDK_Client)
            // Debug wireframe colors — green for local shapes, orange for
            // network-replicated (server-authoritative) shapes.
            inline static godot::Color local_debug_color      = godot::Color(0, 1, 0);
            inline static godot::Color replicated_debug_color = godot::Color(1, 0.55f, 0);
            inline static bool default_debug_enabled = false;
            #endif


            // Hooks //
            void _notification(int what);


            // Managers //
            static Collision_Shape* create(godot::Node3D* owner);
            void destroy();
            // Applies a shape via Godot's native set_shape() AND (client-only)
            // keeps current_shape + the debug wireframe in sync. Every path that
            // assigns a shape should go through this instead of set_shape().
            void assign_shape(godot::Ref<godot::Shape3D> shape);
            #if defined(VSDK_Client)
            void refresh_debug_mesh();
            #endif


            // Getters //
            // net_id of the owning synced body, or 0 if local-only / no parent.
            uint32_t get_parent_net_id() const;
            // True when the owning body is server-authoritative (replicated).
            bool is_replicated() const;
            bool is_debug_visible() const;


            // Setters //
            void set_debug_visible(bool state);
            // Client-only global toggle for every live shape + default for new ones.
            static void set_debug_all(bool state);
    };
}
