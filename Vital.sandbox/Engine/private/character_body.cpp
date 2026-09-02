/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: character_body.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Character Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/character_body.h>
#include <Vital.sandbox/Manager/public/network.h>


////////////////////////////////////
// Vital: Engine: Character_Body //
////////////////////////////////////

namespace Vital::Engine {
    // Hooks //
    void Character_Body::_ready() { 
        _ready_sync(pending_authority); 
    }

    void Character_Body::_process(double delta) {
        on_sync_process(delta); 
    }


    // Managers //
    Character_Body* Character_Body::create(int authority_peer) {
        auto body = memnew(Character_Body);
        body -> setup_create(authority_peer);
        return body;
    }

    void Character_Body::destroy() { 
        setup_destroy(); 
    }


    // Getters //
    Engine::PhysicsType Character_Body::get_physics_type() const { 
        return PhysicsType::Character;
    }
}
