/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: rigid_body.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rigid Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/rigid_body.h>
#include <Vital.sandbox/Manager/public/network.h>


////////////////////////////////
// Vital: Engine: Rigid_Body //
////////////////////////////////

namespace Vital::Engine {
    // Hooks //
    void Rigid_Body::_ready() { 
        _ready_sync(pending_authority); 
    }

    void Rigid_Body::_process(double delta) { 
        on_sync_process(delta); 
    }


    // Managers //
    Rigid_Body* Rigid_Body::create(int authority_peer) {
        auto body = memnew(Rigid_Body);
        body -> setup_create(authority_peer);
        return body;
    }

    void Rigid_Body::destroy() { 
        setup_destroy(); 
    }


    // Getters //
    Engine::PhysicsType Rigid_Body::get_physics_type() const { 
        return PhysicsType::Rigid; 
    }
}
