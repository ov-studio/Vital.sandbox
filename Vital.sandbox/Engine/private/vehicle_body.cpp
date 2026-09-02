/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: vehicle_body.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vehicle Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/vehicle_body.h>
#include <Vital.sandbox/Manager/public/network.h>


//////////////////////////////////
// Vital: Engine: Vehicle_Body //
//////////////////////////////////

namespace Vital::Engine {
    // Hooks //
    void Vehicle_Body::_ready() { 
        _ready_sync(pending_authority); 
    }

    void Vehicle_Body::_process(double delta) { 
        on_sync_process(delta); 
    }


    // Managers //
    Vehicle_Body* Vehicle_Body::create(int authority_peer) {
        auto body = memnew(Vehicle_Body);
        body -> setup_create(authority_peer);
        return body;
    }

    void Vehicle_Body::destroy() { 
        setup_destroy();
    }


    // Getters //
    Engine::PhysicsType Vehicle_Body::get_physics_type() const { 
        return PhysicsType::Vehicle; 
    }
}
