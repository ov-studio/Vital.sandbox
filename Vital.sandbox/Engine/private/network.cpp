/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: network.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/network.h>
#include <Vital.sandbox/Engine/public/model.h>
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Manager/public/asset.h>


/////////////////////////////
// Vital: Engine: Network //
/////////////////////////////

namespace Vital::Engine {

    void Network::setup_rpc() {
        // Reliable channel 0 — events, spawn, destroy.
        godot::Dictionary reliable;
        reliable["rpc_mode"]      = (int)godot::MultiplayerAPI::RPC_MODE_ANY_PEER;
        reliable["transfer_mode"] = (int)godot::MultiplayerPeer::TRANSFER_MODE_RELIABLE;
        reliable["call_local"]    = false;
        reliable["channel"]       = 0;
        rpc_config("_receive",      reliable);
        rpc_config("_spawn_model",  reliable);
        rpc_config("_destroy_model",reliable);
        rpc_config("_sync_state",   reliable);  // late-join full snapshot, must arrive
        rpc_config("_set_authority",reliable);  // authority change, must arrive

        // Unreliable ordered channel 0 — per-frame position/rotation sync.
        // We stay on channel 0 and use UNRELIABLE_ORDERED so Godot's RPC layer
        // routes it correctly without conflicting with scene_cache_interface.
        godot::Dictionary unreliable;
        unreliable["rpc_mode"]      = (int)godot::MultiplayerAPI::RPC_MODE_ANY_PEER;
        unreliable["transfer_mode"] = (int)godot::MultiplayerPeer::TRANSFER_MODE_UNRELIABLE_ORDERED;
        unreliable["call_local"]    = false;
        unreliable["channel"]       = 0;
        rpc_config("_sync_models",  unreliable);  // server -> all clients
        rpc_config("_sync_client",  unreliable);  // authority client -> server
    }

    // Received on clients — batch of model transforms from server.
    void Network::_sync_models(godot::PackedByteArray data) {
        Manager::Network::get_singleton()->dispatch_sync_batch(data, false);
    }

    // Received on clients — reliable full state dump for late-joiners.
    void Network::_sync_state(godot::PackedByteArray data) {
        Manager::Network::get_singleton()->dispatch_sync_batch(data, true);
    }

    // Received on server — client-authority position upload.
    void Network::_sync_client(godot::PackedByteArray data) {
        #if !defined(VSDK_Client)
        auto tree = godot::Object::cast_to<godot::SceneTree>(
            godot::Engine::get_singleton()->get_main_loop());
        int sender = tree ? tree->get_multiplayer()->get_remote_sender_id() : 0;
        Manager::Network::get_singleton()->dispatch_client_sync(data, sender);
        #endif
    }

    void Network::_receive(godot::Dictionary data) {
        Manager::Network::get_singleton()->_on_packet_received(data);
    }

    // Invoked on every client by the server when a model is created.
    // net_id is pre-assigned so _ready() sees it immediately.
    void Network::_spawn_model(int net_id, godot::String name, int authority) {
        #if defined(VSDK_Client)
        std::string model_name = Tool::to_std_string(name);
        auto it = Model::cache_loaded.find(model_name);

        Model* object = memnew(Model);
        object->net_id            = (uint32_t)net_id;
        object->pending_authority = authority;
        object->remote            = true;

        if (it != Model::cache_loaded.end()) {
            godot::Node* instance = it->second->instantiate();
            if (instance) object->add_child(instance);
            object->set_model_name(model_name);
        } else {
            // Asset not downloaded yet — create placeholder, hydrate later.
            object->set_model_name(model_name);
            object->placeholder = true;
            object->set_visible(false);
            Manager::Asset::get_singleton()->queue_spawn(model_name, object);
        }

        Core::get_singleton()->add_child(object);

        // Register with the network sync registry.
        Manager::Network::get_singleton()->enqueue_model_registration(object);

        if (Model::on_spawned_callback) Model::on_spawned_callback(object, true);
        godot::UtilityFunctions::print("_spawn_model: net_id=", net_id, " name=", name);
        #endif
    }

    // Received on all clients — updates model authority and resets interpolation state.
    void Network::_set_authority(int net_id, int peer_id) {
        #if defined(VSDK_Client)
        auto core = Core::get_singleton();
        if (!core) return;
        for (int i = 0; i < core->get_child_count(); i++) {
            Model* m = godot::Object::cast_to<Model>(core->get_child(i));
            if (!m || m->get_net_id() != (uint32_t)net_id) continue;
            m->sync_authority = peer_id;
            // Reset snapshot buffer — stale snapshots from old authority
            // must not bleed into the new authority's interpolation.
            m->snap_head        = 0;
            m->snap_count       = 0;
            m->snap_clock       = 0.0f;
            m->interp_ready     = false;
            m->jitter_last_arrival = -1.0f;
            m->jitter_idx       = 0;
            m->jitter_count     = 0;
            m->adaptive_delay   = Engine::Model::BUFFER_DELAY;
            for (int i = 0; i < Engine::Model::JITTER_WINDOW; i++)
                m->jitter_intervals[i] = 0.0f;
            godot::UtilityFunctions::print("_set_authority: net_id=", net_id,
                " -> peer_id=", peer_id);
            return;
        }
        #endif
    }

    // Invoked on every client by the server when a model is destroyed.
    void Network::_destroy_model(int net_id) {
        #if defined(VSDK_Client)
        auto core = Core::get_singleton();
        if (!core) return;
        for (int i = 0; i < core->get_child_count(); i++) {
            Model* m = godot::Object::cast_to<Model>(core->get_child(i));
            if (m && m->get_net_id() == (uint32_t)net_id) {
                m->queue_free();
                godot::UtilityFunctions::print("_destroy_model: net_id=", net_id);
                return;
            }
        }
        #endif
    }

    #if defined(VSDK_Client)
    void Network::_on_connected_to_server()  { if (on_connected_to_server)  on_connected_to_server();  }
    void Network::_on_connection_failed()    { if (on_connection_failed)    on_connection_failed();    }
    void Network::_on_server_disconnected()  { if (on_server_disconnected)  on_server_disconnected();  }
    #else
    void Network::_on_peer_connected(int id)    { if (on_peer_connected)    on_peer_connected(id);    }
    void Network::_on_peer_disconnected(int id) { if (on_peer_disconnected) on_peer_disconnected(id); }
    #endif
}
