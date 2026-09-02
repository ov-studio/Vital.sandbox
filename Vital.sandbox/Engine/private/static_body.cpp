/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: static_body.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Static Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/static_body.h>
#include <Vital.sandbox/Manager/public/network.h>


/////////////////////////////////
// Vital: Engine: Static_Body //
/////////////////////////////////

namespace Vital::Engine {
    // Hooks //
    void Static_Body::_ready() { 
        _ready_sync(pending_authority); 
    }

    void Static_Body::_process(double delta) { 
        on_sync_process(delta); 
    }


    // Managers //
    Static_Body* Static_Body::create(int authority_peer) {
        auto body = memnew(Static_Body);
        body -> setup_create(authority_peer);
        return body;
    }

    void Static_Body::destroy() { 
        setup_destroy(); 
    }


    // Getters //
    Engine::PhysicsType Static_Body::get_physics_type() const { 
        return PhysicsType::Static; 
    }
}
