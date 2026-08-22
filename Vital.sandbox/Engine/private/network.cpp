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
        // Channel 0, reliable — used for RPC events and model spawn/destroy signals.
        godot::Dictionary cfg;
        cfg["rpc_mode"]     = (int)godot::MultiplayerAPI::RPC_MODE_ANY_PEER;
        cfg["transfer_mode"]= (int)godot::MultiplayerPeer::TRANSFER_MODE_RELIABLE;
        cfg["call_local"]   = false;
        cfg["channel"]      = 0;
        rpc_config("_receive",      cfg);
        rpc_config("_spawn_model",  cfg);
        rpc_config("_destroy_model",cfg);
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

        if (Model::on_spawned_callback) Model::on_spawned_callback(object, true);
        godot::UtilityFunctions::print("_spawn_model: net_id=", net_id, " name=", name);
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
