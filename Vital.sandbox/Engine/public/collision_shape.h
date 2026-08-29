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

namespace Vital::Engine {
    class Collision_Shape : public godot::CollisionShape3D {
        GDCLASS(Collision_Shape, godot::CollisionShape3D)
        private:
            // Instantiators //
            Collision_Shape() = default;
            ~Collision_Shape() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static Collision_Shape* create(godot::Node3D* owner);
            void destroy();
    };
}
