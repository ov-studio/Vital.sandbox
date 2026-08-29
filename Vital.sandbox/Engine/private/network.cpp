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
#include <Vital.sandbox/Engine/public/rigid_body.h>
#include <Vital.sandbox/Engine/public/static_body.h>
#include <Vital.sandbox/Engine/public/character_body.h>
#include <Vital.sandbox/Engine/public/animatable_body.h>
#include <Vital.sandbox/Engine/public/vehicle_body.h>
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Manager/public/asset.h>


/////////////////////////////
// Vital: Engine: Network //
/////////////////////////////

namespace Vital::Engine {
    // Managers //
    void Network::setup_rpc() {
        godot::Dictionary reliable;
        reliable["rpc_mode"] = (int)godot::MultiplayerAPI::RPC_MODE_ANY_PEER;
        reliable["transfer_mode"] = (int)godot::MultiplayerPeer::TRANSFER_MODE_RELIABLE;
        reliable["call_local"] = false;
        reliable["channel"] = 0;
        rpc_config("_receive", reliable);
        rpc_config("_spawn_entity", reliable);
        rpc_config("_destroy_entity", reliable);
        rpc_config("_sync_state", reliable);
        rpc_config("_set_authority", reliable);

        godot::Dictionary unreliable;
        unreliable["rpc_mode"] = (int)godot::MultiplayerAPI::RPC_MODE_ANY_PEER;
        unreliable["transfer_mode"] = (int)godot::MultiplayerPeer::TRANSFER_MODE_UNRELIABLE_ORDERED;
        unreliable["call_local"] = false;
        unreliable["channel"] = 0;
        rpc_config("_sync_entities", unreliable);
        rpc_config("_sync_client", unreliable);
    }


    // Signals //
    void Network::_receive(godot::Dictionary data) {
        Manager::Network::get_singleton() -> _on_packet_received(data);
    }
    
    void Network::_sync_entities(godot::PackedByteArray data) {
        Manager::Network::get_singleton() -> dispatch_sync_batch(data, false);
    }

    void Network::_sync_state(godot::PackedByteArray data) {
        Manager::Network::get_singleton() -> dispatch_sync_batch(data, true);
    }

    void Network::_sync_client(godot::PackedByteArray data) {
        #if !defined(VSDK_Client)
        auto tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton() -> get_main_loop());
        int sender = tree ? tree -> get_multiplayer() -> get_remote_sender_id() : 0;
        Manager::Network::get_singleton() -> dispatch_client_sync(data, sender);
        #endif
    }

    void Network::_spawn_entity(int net_id, int type_id, godot::String name, int authority) {
        #if defined(VSDK_Client)
        using ST = Engine::ISyncable::SyncType;
        switch (static_cast<ST>(type_id)) {
            case ST::Model: {
                std::string model_name = Tool::to_std_string(name);
                auto it = Engine::Model::cache_loaded.find(model_name);
                Engine::Model* object = memnew(Engine::Model);
                object -> net_id = (uint32_t)net_id;
                object -> pending_authority = authority;
                object -> remote = true;

                if (it != Engine::Model::cache_loaded.end()) {
                    godot::Node* instance = it -> second -> instantiate();
                    if (instance) object -> add_child(instance);
                    object -> set_model_name(model_name);
                } 
                else {
                    object -> set_model_name(model_name);
                    object -> placeholder = true;
                    object -> set_visible(false);
                    Manager::Asset::get_singleton() -> queue_spawn(model_name, object);
                }
                Engine::Core::get_singleton() -> add_child(object);
                Manager::Network::get_singleton() -> enqueue_syncable_registration(object);
                if (Engine::Model::on_spawned_callback) Engine::Model::on_spawned_callback(object, true);
                godot::UtilityFunctions::print("_spawn_entity [Model]: net_id=", net_id, " name=", name);
                break;
            }
            case ST::PhysicsBody: {
                std::string sub = Tool::to_std_string(name);
                Engine::ISyncable* entity = nullptr;
                Engine::PhysicsSubType sub_type = Engine::PhysicsSubType::Rigid;  // TODO: empty no init maybe?

                if (sub == "rigid") {
                    sub_type = Engine::PhysicsSubType::Rigid;
                    auto body = memnew(Engine::Rigid_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else if (sub == "static") {
                    sub_type = Engine::PhysicsSubType::Static;
                    auto body = memnew(Engine::Static_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else if (sub == "character") {
                    sub_type = Engine::PhysicsSubType::Character;
                    auto body = memnew(Engine::Character_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else if (sub == "animatable") {
                    sub_type = Engine::PhysicsSubType::Animatable;
                    auto body = memnew(Engine::Animatable_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else if (sub == "vehicle") {
                    sub_type = Engine::PhysicsSubType::Vehicle;
                    auto body = memnew(Engine::Vehicle_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else {
                    godot::UtilityFunctions::push_warning("_spawn_entity [PhysicsBody]: unknown sub-type=", name);
                    break;
                }

                if (entity) {
                    entity -> net_id = (uint32_t)net_id;
                    entity -> sync_authority = authority;
                    entity -> reset_sync_state();
                    Manager::Network::get_singleton() -> enqueue_syncable_registration(entity);
                    godot::UtilityFunctions::print("_spawn_entity [PhysicsBody/", name, "]: net_id=", net_id);

                    // TODO: SHARE IN BETTER WAY?
                    // Notify Lua so it can hydrate collision shapes / wheels on
                    // this remote body — mirrors Model::on_spawned_callback.
                    if (Engine::on_physics_body_spawned_callback)
                        Engine::on_physics_body_spawned_callback(entity, sub_type, true);
                }
                break;
            }
            default: {
                godot::UtilityFunctions::push_warning("_spawn_entity: unknown type_id=", type_id);
                break;
            }
        }
        #endif
    }

    void Network::_set_authority(int net_id, int peer_id) {
        #if defined(VSDK_Client)
        auto core = Engine::Core::get_singleton();
        if (!core) return;
        Engine::ISyncable* entity = Manager::Network::get_singleton() -> find_syncable((uint32_t)net_id);
        if (!entity) return;
        entity -> set_sync_authority(peer_id);
        godot::UtilityFunctions::print("_set_authority: net_id=", net_id, " -> peer_id=", peer_id);
        #endif
    }

    void Network::_destroy_entity(int net_id) {
        #if defined(VSDK_Client)
        Engine::ISyncable* entity = Manager::Network::get_singleton() -> find_syncable((uint32_t)net_id);
        if (!entity) return;
        entity -> destroy_sync();
        godot::UtilityFunctions::print("_destroy_entity: net_id=", net_id);
        #endif
    }

    #if defined(VSDK_Client)
    void Network::_on_connected_to_server() { if (on_connected_to_server) on_connected_to_server(); }
    void Network::_on_connection_failed() { if (on_connection_failed) on_connection_failed(); }
    void Network::_on_server_disconnected() { if (on_server_disconnected) on_server_disconnected(); }
    #else
    void Network::_on_peer_connected(int id) { if (on_peer_connected) on_peer_connected(id); }
    void Network::_on_peer_disconnected(int id) { if (on_peer_disconnected) on_peer_disconnected(id); }
    #endif
}
