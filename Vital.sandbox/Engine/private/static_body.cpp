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
        #if !defined(VSDK_Client)
        if (authority_peer != 0) {
            body->net_id            = body->next_net_id++;
            body->pending_authority = authority_peer;
            uint32_t captured_id    = body->net_id;
            int captured_auth       = authority_peer;
            Core::get_singleton()->add_child(body);
            Core::get_singleton()->enqueue([body, captured_id, captured_auth]() {
                Manager::Network::get_singleton()->enqueue_syncable_registration(body);
                auto net_node = Manager::Network::get_singleton()->get_node();
                if (net_node) {
                    net_node->rpc("_spawn_entity",
                        (int)captured_id,
                        (int)ISyncable::SyncType::PhysicsBody,
                        godot::String("static"),
                        captured_auth);
                }
            });
        } else {
            Core::get_singleton()->add_child(body);
        }
        #else
        Core::get_singleton()->add_child(body);
        #endif
        return body;
    }

    void Static_Body::destroy() {
        #if !defined(VSDK_Client)
        if (net_id != 0) {
            auto net_node = Manager::Network::get_singleton()->get_node();
            if (net_node) net_node->rpc("_destroy_entity", (int)net_id);
        }
        #endif
        queue_free();
    }


    // Getters //
    Engine::PhysicsType get_physics_type() const { 
        return PhysicsType::Static; 
    }
}
