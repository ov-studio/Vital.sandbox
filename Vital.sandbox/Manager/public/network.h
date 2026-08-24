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

// TOOD: Improve

namespace Vital::Engine { class Model; }

namespace Vital::Manager {
    class Network : public godot::Node, public Tool::Base<Network> {
        friend class Tool::Base<Network>;
        public:
            static constexpr const char* Name = "Network.manager";

            // ENet channel assignments:
            //   0 — reliable RPC (events, handshakes)     TRANSFER_MODE_RELIABLE
            //   1 — unreliable sync (position/rotation)   TRANSFER_MODE_UNRELIABLE_ORDERED
            static constexpr int CHANNEL_RPC  = 0;

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

            // Persistent net_id -> Model* map — updated on register/unregister,
            // used by dispatch_client_sync and dispatch_sync_batch for O(1) lookup
            // without rebuilding a temporary map every packet.
            std::unordered_map<uint32_t, Engine::Model*> sync_id_map;

            // Pending registration queue — models added from enqueue() callbacks
            // post to this instead of directly into sync_models, avoiding the
            // O(N) child-scan in poll().
            std::vector<Engine::Model*> sync_pending;
            std::mutex sync_pending_mutex;

            // Per-frame dirty batch buffer reused across frames (avoids realloc).
            godot::PackedByteArray sync_batch_buf;

            // Sync interval in seconds — set from config on host(), read each poll().
            // Default 1/20 = 20 Hz. Configurable via network.sync_rate in config.yaml.
            float sync_interval = 1.0f / 20.0f;



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



        public:
            Network() = default;
            ~Network() = default;


            // Managers //
            void teardown();
            void _on_packet_received(godot::Dictionary data);


            // Model sync registry //
            void register_model(Engine::Model* model);
            void unregister_model(Engine::Model* model);
            // Called from deferred enqueue() context — posts to pending queue
            // instead of directly modifying sync_models under poll().
            void enqueue_model_registration(Engine::Model* model);


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

            // Sync transport — routed through Godot's RPC layer (avoids scene_cache conflict).
            bool broadcast_sync(const godot::PackedByteArray& data);      // server -> all clients (unreliable)
            bool send_sync_to_server(const godot::PackedByteArray& data);  // client -> server (unreliable)

            // Inbound dispatch — called by Engine::Network RPC handlers.
            void dispatch_sync_batch(const godot::PackedByteArray& data, bool is_state_dump);
            void dispatch_client_sync(const godot::PackedByteArray& data, int sender_id);


            #if !defined(VSDK_Client)
            void send_full_state_to_peer(int peer_id);
            #endif

            // Expose the Engine::Network node so Model can call rpc() on it.
            Engine::Network* get_node() const { return node; }

            void poll(double delta = 0.0);
    };
}
