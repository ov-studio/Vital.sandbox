/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: network.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/network.h>
#if !defined(VSDK_Client)
#include <Vital.sandbox/Config/server.h>
#endif


//////////////////////////////
// Vital: Manager: Network //
//////////////////////////////

namespace Vital::Engine { class Model; }

namespace Vital::Manager {
    // TODO: Improve
    class Network : public godot::Node, public Tool::Base<Network> {
        friend class Tool::Base<Network>;
        public:
            static constexpr const char* Name = "Network.manager";

            // ENet channel assignments:
            //   0 — reliable RPC (events, handshakes)     TRANSFER_MODE_RELIABLE
            //   1 — unreliable sync (position/rotation)   TRANSFER_MODE_UNRELIABLE_ORDERED
            static constexpr int CHANNEL_RPC  = 0;
            static constexpr int CHANNEL_SYNC = 1;

            // Magic for batched late-join state dump packets ("VSST").
            // Defined here so both server (sender) and client (receiver) see it.
            static constexpr uint32_t STATE_DUMP_MAGIC = 0x56535354u;

        private:
            godot::Ref<godot::ENetMultiplayerPeer> peer;
            Engine::Network* node = nullptr;

            // Model sync registry — every live Model registers here so
            // poll() can drive sync_tick() without touching the scene tree.
            std::vector<Engine::Model*> sync_models;
            std::mutex sync_models_mutex;

            #if defined(VSDK_Client)
            bool auto_reconnect    = false;
            bool pending_handshake = false;
            std::string reconnect_ip;
            int   reconnect_port     = 0;
            int   reconnect_attempts = 0;
            int   reconnect_max      = 5;
            float reconnect_delay    = 3.0f;
            float reconnect_timer    = 0.0f;
            #else
            std::unordered_set<int> connected_peers;
            const Config::Server* server_config = nullptr;
            std::string server_ip;
            #endif

            static godot::SceneTree* get_scene_tree();
            void create();
            void destroy();
            void wire_signals();
            void unwire_signals();

            // Low-level raw packet send on CHANNEL_SYNC (unreliable-ordered).
            // peerID == 0 => broadcast to all connected peers (server-side).
            // peerID == 1 => send to server (client-side).
            bool send_raw(const godot::PackedByteArray& data, int peerID, bool unreliable);

            // Drain inbound raw sync packets and route them to the correct Model.
            void drain_sync_packets();

        public:
            Network() = default;
            ~Network() = default;


            // Managers //
            void teardown();
            void _on_packet_received(godot::Dictionary data);


            // Model sync registry //
            void register_model(Engine::Model* model);
            void unregister_model(Engine::Model* model);


            // State //
            bool is_connected() const;
            bool is_connecting() const;
            int  get_peer_id() const;

            #if defined(VSDK_Client)
            bool connect_to_server(const std::string& ip, int port, bool enable_reconnect = false);
            bool disconnect_from_server();
            void _on_connected_to_server();
            void _on_connection_failed();
            void _on_server_disconnected();
            void set_reconnect_config(int max_attempts, float delay_seconds);
            void _schedule_reconnect();
            std::string get_server_ip() const;
            #else
            bool host(Config::Server& config);
            bool close();
            void _on_peer_connected(int id);
            void _on_peer_disconnected(int id);
            const std::unordered_set<int>& get_connected_peers() const;
            int  get_peer_count() const;
            const Config::Server& get_server_config() const;
            std::string get_server_ip() const;
            #endif


            // Shared RPC (channel 0, reliable) //
            bool send(const Tool::Stack& stack, int peerID = 0);
            bool broadcast(const Tool::Stack& stack);
            bool send_to_server(const Tool::Stack& stack);

            // Sync transport (channel 1, unreliable-ordered) //
            // Called by Model::sync_tick internally.
            bool broadcast_sync(const godot::PackedByteArray& data);      // server -> all clients
            bool send_sync_to_server(const godot::PackedByteArray& data);  // client -> server

            #if !defined(VSDK_Client)
            // Late-join full state dump — sends the current transform of every
            // registered model to peer_id via the reliable channel so they arrive
            // guaranteed and in-order after the MultiplayerSpawner spawn signals.
            void send_full_state_to_peer(int peer_id);
            #endif

            // Expose the Engine::Network node so Model can call rpc() on it.
            Engine::Network* get_node() const { return node; }

            void poll(double delta = 0.0);
    };
}
