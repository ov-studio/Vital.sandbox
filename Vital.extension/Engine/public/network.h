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
#include <Vital.extension/Engine/public/console.h>


/////////////////////////////
// Vital: Engine: Network //
/////////////////////////////

namespace Vital::Engine {
    // Bridge node — registered in GDExtension _register_types
    class NetworkBridge : public godot::Node {
        GDCLASS(NetworkBridge, godot::Node)
        public:
            static void _bind_methods();

            std::function<void(int)> on_peer_connected;
            std::function<void(int)> on_peer_disconnected;
            std::function<void()>    on_connected_to_server;
            std::function<void()>    on_connection_failed;
            std::function<void()>    on_server_disconnected;

            void _on_peer_connected(int id);
            void _on_peer_disconnected(int id);
            void _on_connected_to_server();
            void _on_connection_failed();
            void _on_server_disconnected();
    };

    class Network {
        private:
            inline static Network* singleton = nullptr;
            godot::Ref<godot::ENetMultiplayerPeer> peer;
            NetworkBridge* bridge = nullptr;
            std::unordered_set<int> connected_peers;

            bool        auto_reconnect     = false;
            std::string reconnect_ip;
            int         reconnect_port     = 0;
            int         reconnect_attempts = 0;
            int         reconnect_max      = 5;
            float       reconnect_delay    = 3.0f;
            float       reconnect_timer    = 0.0f;

            static godot::SceneTree* get_scene_tree();
            static Vital::Tool::Stack make_stack(int32_t id);

            void create_bridge();
            void destroy_bridge();
            void wire_server_signals();
            void wire_client_signals();
            void unwire_signals();

        public:
            Network() = default;
            ~Network() = default;

            static Network* get_singleton();
            static void free_singleton();

            // State
            bool is_active() const;
            bool is_connecting() const;
            static bool is_server();
            int get_peer_id() const;
            const std::unordered_set<int>& get_connected_peers() const;
            int get_peer_count() const;
            void print_status() const;

            // Server
            bool host(int port, int max_clients = 32);
            bool close();
            void _on_peer_connected(int id);
            void _on_peer_disconnected(int id);

            // Client
            bool connect_to_server(const std::string& ip, int port, bool enable_reconnect = false);
            bool disconnect_from_server();
            void _on_connected_to_server();
            void _on_connection_failed();
            void _on_server_disconnected();

            // Reconnect
            void set_reconnect_config(int max_attempts, float delay_seconds);
            void _schedule_reconnect();

            // Send / Receive
            bool send(const Vital::Tool::Stack& stack, int peerID = 0, bool isLatent = false);
            bool broadcast(const Vital::Tool::Stack& stack, bool isLatent = false);
            bool send_to_server(const Vital::Tool::Stack& stack, bool isLatent = false);

            // Poll
            void poll(double delta = 0.0);

            // Emit outbound
            static void emit(Vital::Tool::Stack& arguments, int peerID = 0, bool isLatent = false);
    };
}