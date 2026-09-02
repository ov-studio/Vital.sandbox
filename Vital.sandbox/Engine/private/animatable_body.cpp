/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: animatable_body.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Animatable Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/animatable_body.h>
#include <Vital.sandbox/Manager/public/network.h>


/////////////////////////////////////
// Vital: Engine: Animatable_Body //
/////////////////////////////////////

namespace Vital::Engine {
    // Hooks //
    void Animatable_Body::_ready() { 
        _ready_sync(pending_authority); 
    }

    void Animatable_Body::_process(double delta) { 
        on_sync_process(delta); 
    }


    // Managers //
    Animatable_Body* Animatable_Body::create(int authority_peer) {
        auto body = memnew(Animatable_Body);
        body -> setup_create(authority_peer);
        return body;
    }

    void Animatable_Body::destroy() { 
        setup_destroy(); 
    }


    // Getters //
    Engine::PhysicsType Animatable_Body::get_physics_type() const { 
        return PhysicsType::Animatable; 
    }
}
