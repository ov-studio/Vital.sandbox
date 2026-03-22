/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: network.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/index.h>
#include <godot_cpp/classes/e_net_multiplayer_peer.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/multiplayer_peer.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>


/////////////////////////////
// Vital: Engine: Network //
/////////////////////////////

namespace Vital::Engine {
    // TODO: Improve

    // NetworkNode — lives in the SceneTree so Godot RPC can route packets.
    // _receive() is the single RPC entry point for all incoming packets.
    class NetworkNode : public godot::Node {
        GDCLASS(NetworkNode, godot::Node)

        public:
            static void _bind_methods();

            // Called by Godot's RPC system when a packet arrives
            void _receive(godot::Dictionary data);

            // Called once after being added to the scene tree
            // to register RPC config (requires a valid node path)
            void setup_rpc();

            #if defined(Vital_SDK_Client)
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


    class Network {
        private:
            inline static Network* singleton = nullptr;
            godot::Ref<godot::ENetMultiplayerPeer> peer;
            NetworkNode* node = nullptr;

            #if defined(Vital_SDK_Client)
            bool        auto_reconnect     = false;
            bool        pending_handshake  = false;
            std::string reconnect_ip;
            int         reconnect_port     = 0;
            int         reconnect_attempts = 0;
            int         reconnect_max      = 5;
            float       reconnect_delay    = 3.0f;
            float       reconnect_timer    = 0.0f;
            #else
            std::unordered_set<int> connected_peers;
            #endif

            static godot::SceneTree* get_scene_tree();

            void create_node();
            void destroy_node();

            #if defined(Vital_SDK_Client)
            void wire_client_signals();
            #else
            void wire_server_signals();
            #endif

            void unwire_signals();

        public:
            Network()  = default;
            ~Network() = default;

            static Network* get_singleton();
            static void free_singleton();

            // Called by NetworkNode::_receive when an RPC packet arrives
            void _on_packet_received(godot::Dictionary data);

            // State
            bool is_active() const;
            bool is_connecting() const;
            static bool is_server();
            int get_peer_id() const;
            void print_status() const;

            #if defined(Vital_SDK_Client)
            bool connect_to_server(const std::string& ip, int port, bool enable_reconnect = false);
            bool disconnect_from_server();
            void _on_connected_to_server();
            void _on_connection_failed();
            void _on_server_disconnected();
            void set_reconnect_config(int max_attempts, float delay_seconds);
            void _schedule_reconnect();
            #else
            bool host(int port, int max_clients = 32);
            bool close();
            void _on_peer_connected(int id);
            void _on_peer_disconnected(int id);
            const std::unordered_set<int>& get_connected_peers() const;
            int get_peer_count() const;
            #endif

            // Shared — Stack-based API (same as before, RPC transport is internal)
            bool send(const Vital::Tool::Stack& stack, int peerID = 0);
            bool broadcast(const Vital::Tool::Stack& stack);
            bool send_to_server(const Vital::Tool::Stack& stack);
            void poll(double delta = 0.0);
            static void emit(Vital::Tool::Stack& arguments, int peerID = 0);
    };
}