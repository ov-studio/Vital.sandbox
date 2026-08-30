/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: network.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/syncable.h>


/////////////////////////////
// Vital: Engine: Network //
/////////////////////////////

namespace Vital::Engine {
    class Network : public godot::Node {
        GDCLASS(Network, godot::Node)
        private:
            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("setup_rpc"), &Network::setup_rpc);
                godot::ClassDB::bind_method(godot::D_METHOD("_receive", "data"), &Network::_receive);
                godot::ClassDB::bind_method(godot::D_METHOD("_spawn_entity", "net_id", "type_id", "name", "authority"), &Network::_spawn_entity);
                godot::ClassDB::bind_method(godot::D_METHOD("_destroy_entity", "net_id"), &Network::_destroy_entity);
                godot::ClassDB::bind_method(godot::D_METHOD("_set_authority", "net_id", "peer_id"), &Network::_set_authority);
                godot::ClassDB::bind_method(godot::D_METHOD("_sync_entities", "data"), &Network::_sync_entities);
                godot::ClassDB::bind_method(godot::D_METHOD("_sync_state", "data"), &Network::_sync_state);
                godot::ClassDB::bind_method(godot::D_METHOD("_sync_client", "data"), &Network::_sync_client);
                godot::ClassDB::bind_method(godot::D_METHOD("_sync_shape", "net_id", "shape_type", "params"), &Network::_sync_shape);
                godot::ClassDB::bind_method(godot::D_METHOD("_spawn_wheel", "net_id", "wheel_index", "position", "rotation"), &Network::_spawn_wheel);
                godot::ClassDB::bind_method(godot::D_METHOD("_destroy_wheel", "net_id", "wheel_index"), &Network::_destroy_wheel);
                godot::ClassDB::bind_method(godot::D_METHOD("_sync_wheel_config", "net_id", "wheel_index", "key", "value"), &Network::_sync_wheel_config);
                godot::ClassDB::bind_method(godot::D_METHOD("_sync_wheel_transform", "net_id", "wheel_index", "position", "rotation"), &Network::_sync_wheel_transform);
                #if defined(VSDK_Client)
                godot::ClassDB::bind_method(godot::D_METHOD("_on_connected_to_server"), &Network::_on_connected_to_server);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_connection_failed"), &Network::_on_connection_failed);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_server_disconnected"), &Network::_on_server_disconnected);
                #else
                godot::ClassDB::bind_method(godot::D_METHOD("_on_peer_connected", "id"), &Network::_on_peer_connected);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_peer_disconnected", "id"), &Network::_on_peer_disconnected);
                #endif
            }
        public:
            // Managers //
            void setup_rpc();


            // Signals //
            void _receive(godot::Dictionary data);
            void _spawn_entity(int net_id, int type_id, godot::String name, int authority);
            void _destroy_entity(int net_id);
            void _set_authority(int net_id, int peer_id);
            void _sync_entities(godot::PackedByteArray data);
            void _sync_state(godot::PackedByteArray data);
            void _sync_client(godot::PackedByteArray data);
            void _sync_shape(int net_id, godot::String shape_type, godot::Array params);
            // Actually finds/creates the Collision_Shape child and applies the shape.
            // Split out of _sync_shape so Manager::Network::poll() can replay a shape
            // sync that arrived before its parent body finished local registration
            // (registration is polled once per frame, not applied the instant
            // _spawn_entity's RPC is handled — see poll()).
            static void apply_shape(uint32_t net_id, godot::String shape_type, godot::Array params);
            void _spawn_wheel(int net_id, int wheel_index, godot::Vector3 position, godot::Vector3 rotation);
            void _destroy_wheel(int net_id, int wheel_index);
            void _sync_wheel_config(int net_id, int wheel_index, godot::String key, godot::Variant value);
            void _sync_wheel_transform(int net_id, int wheel_index, godot::Vector3 position, godot::Vector3 rotation);
            #if defined(VSDK_Client)
            std::function<void()> on_connected_to_server;
            std::function<void()> on_connection_failed;
            std::function<void()> on_server_disconnected;
            void _on_connected_to_server();
            void _on_connection_failed();
            void _on_server_disconnected();
            #else
            std::function<void(int)> on_peer_connected;
            std::function<void(int)> on_peer_disconnected;
            void _on_peer_connected(int id);
            void _on_peer_disconnected(int id);
            #endif
    };
}
