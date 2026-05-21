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
#if !defined(Vital_SDK_Client)
#include <Vital.sandbox/Engine/public/cfg_server.h>
#endif


/////////////////////////////
// Vital: Engine: Network //
/////////////////////////////

namespace Vital::Engine {
    // TODO: Improve

    class NetworkNode : public godot::Node {
        GDCLASS(NetworkNode, godot::Node)

        public:
            static void _bind_methods();
            void _receive(godot::Dictionary data);
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
            bool auto_reconnect = false;
            bool pending_handshake = false;
            std::string reconnect_ip;
            int reconnect_port = 0;
            int reconnect_attempts = 0;
            int reconnect_max = 5;
            float reconnect_delay = 3.0f;
            float reconnect_timer = 0.0f;
            #else
            std::unordered_set<int> connected_peers;
            const cfg_server* server_config = nullptr;
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
            Network() = default;
            ~Network() = default;

            static Network* get_singleton();
            static void free_singleton();

            void _on_packet_received(godot::Dictionary data);

            // State
            bool is_active() const;
            bool is_connecting() const;
            int get_peer_id() const;

            #if defined(Vital_SDK_Client)
            bool connect_to_server(const std::string& ip, int port, bool enable_reconnect = false);
            bool disconnect_from_server();
            void _on_connected_to_server();
            void _on_connection_failed();
            void _on_server_disconnected();
            void set_reconnect_config(int max_attempts, float delay_seconds);
            void _schedule_reconnect();
            std::string get_server_ip() const;
            #else
            bool host(cfg_server& config);
            bool close();
            void _on_peer_connected(int id);
            void _on_peer_disconnected(int id);
            const std::unordered_set<int>& get_connected_peers() const;
            int get_peer_count() const;
            const cfg_server& get_server_config() const;
            #endif

            // Shared
            bool send(const Tool::Stack& stack, int peerID = 0);
            bool broadcast(const Tool::Stack& stack);
            bool send_to_server(const Tool::Stack& stack);
            void poll(double delta = 0.0);
            static void emit(Tool::Stack& arguments, int peerID = 0);
    };
}