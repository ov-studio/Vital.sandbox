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

namespace Vital::Engine {
    class Collision_Shape : public godot::CollisionShape3D {
        GDCLASS(Collision_Shape, godot::CollisionShape3D)
        friend class Network;
        private:
            #if defined(VSDK_Client)
            inline static bool debug_all  = false;
            inline static std::mutex mutex;
            inline static std::unordered_set<Collision_Shape*> live_instances;
            inline static godot::Color local_debug_color = godot::Color(0, 1, 0);
            inline static godot::Color replicated_debug_color = godot::Color(1, 0.55f, 0);

            godot::Ref<godot::Shape3D> current_shape;
            godot::MeshInstance3D* debug_mesh = nullptr;
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
            // Hooks //
            void _notification(int what);


            // Managers //
            static Collision_Shape* create(godot::Node3D* owner);
            void destroy();
            void assign_shape(godot::Ref<godot::Shape3D> shape);


            // Checkers //
            bool is_replicated() const;
            bool is_debug_visible() const;
            #if defined(VSDK_Client)
            static bool is_debug_all() { return debug_all; }
            #endif


            // Getters //
            uint32_t get_parent_net_id() const;


            // Setters //
            void set_debug_visible(bool state);
            static void set_debug_all(bool state);


            // Misc //
            #if defined(VSDK_Client)
            void refresh_debug_mesh();
            #endif
    };
}