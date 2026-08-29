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

// TODO: Improve

namespace Vital::Engine {
    Vehicle_Body* Vehicle_Body::create(int authority_peer) {
        auto body = memnew(Vehicle_Body);
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
                        godot::String("vehicle"),
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

    void Vehicle_Body::destroy() {
        #if !defined(VSDK_Client)
        if (net_id != 0) {
            auto net_node = Manager::Network::get_singleton()->get_node();
            if (net_node) net_node->rpc("_destroy_entity", (int)net_id);
        }
        #endif
        queue_free();
    }
}
