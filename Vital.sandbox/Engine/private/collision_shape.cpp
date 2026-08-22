/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: collision_shape.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Collision Shape Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/collision_shape.h>


/////////////////////////////////
// Vital: Engine: Collision_Shape //
/////////////////////////////////

namespace Vital::Engine {
    // Managers //
    Collision_Shape* Collision_Shape::create(godot::Node3D* owner) {
        auto body = memnew(Collision_Shape);
        if (owner) owner -> add_child(body);
        else Engine::Core::get_singleton() -> add_child(body);
        return body;
    }

    void Collision_Shape::destroy() {
        queue_free();
    }
}
#endif
