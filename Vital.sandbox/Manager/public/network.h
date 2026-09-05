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
#include <Vital.sandbox/Engine/public/syncable.h>
#if !defined(VSDK_Client)
#include <Vital.sandbox/Config/server.h>
#endif


//////////////////////////////
// Vital: Manager: Network //
//////////////////////////////

// TOOD: Improve

namespace Vital::Engine { class ISyncable; }

namespace Vital::Manager {
    class Network : public godot::Node, public Tool::Base<Network> {
        friend class Tool::Base<Network>;
        public:
            static constexpr const char* Name = "Network.manager";

            // ENet channel assignments:
            //   0 — reliable RPC (events, handshakes)     TRANSFER_MODE_RELIABLE
            //   1 — unreliable sync (position/rotation)   TRANSFER_MODE_UNRELIABLE_ORDERED
            // Magic for batched late-join state dump packets ("VSST").
            // Defined here so both server (sender) and client (receiver) see it.
            static constexpr uint32_t STATE_DUMP_MAGIC = 0x56535354u;
        private:
            godot::Ref<godot::ENetMultiplayerPeer> peer;
            Engine::Network* node = nullptr;

            // Live Model registry — poll() drives sync_tick() through this instead of scanning the scene tree.
            std::vector<Engine::ISyncable*> sync_models;
            std::mutex sync_models_mutex;

            // net_id -> ISyncable* map, kept for O(1) dispatch lookups.
            std::unordered_map<uint32_t, Engine::ISyncable*> sync_id_map;

            // Pending registrations — posted here by enqueue_syncable_registration(),
            // drained into sync_models by poll() instead of an O(N) child-scan.
            std::vector<Engine::ISyncable*> sync_pending;
            std::mutex sync_pending_mutex;

            #if defined(VSDK_Client)
            // Shape syncs (_sync_shape RPC) that arrive before their body finishes
            // local registration. Buffered by net_id, replayed by poll() once that
            // net_id shows up in sync_pending.
            std::unordered_map<uint32_t, std::pair<godot::String, godot::Array>> pending_shape_syncs;
            std::mutex pending_shape_mutex;

            // Same problem for transform (position/rotation/velocity) packets — the
            // state-dump sends can arrive before poll() drains sync_pending. Keeps
            // only the latest state per net_id; applied and cleared on registration.
            std::unordered_map<uint32_t, std::tuple<godot::Vector3, godot::Vector3, godot::Vector3>> pending_transform_syncs;
            std::mutex pending_transform_mutex;

            // Same problem for _reparent_entity RPCs — parent may not be in
            // sync_id_map yet when the RPC arrives. Stores (child_net_id → parent_net_id).
            std::unordered_map<uint32_t, uint32_t> pending_reparent_syncs;
            std::mutex pending_reparent_mutex;
            #endif

            // Per-frame dirty batch buffer reused across frames (avoids realloc).
            godot::PackedByteArray sync_batch_buf;

            // Sync interval in seconds, set from config on host()/apply_sync_config(),
            // read each poll(). Configurable via network.sync_rate in config.yaml.
            float sync_interval = 1.0f / static_cast<float>(Engine::ISyncable::SyncConfig{}.rate);

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

            #if defined(VSDK_Client)
            // Buffer a _reparent_entity for replay once child/parent register.
            void buffer_reparent(uint32_t child_net_id, uint32_t parent_net_id) {
                std::lock_guard<std::mutex> lock(pending_reparent_mutex);
                pending_reparent_syncs[child_net_id] = parent_net_id;
            }
            #endif


            // Managers //
            void teardown();
            void _on_packet_received(godot::Dictionary data);


            // Model sync registry //
            void register_syncable(Engine::ISyncable* entity);
            void unregister_syncable(Engine::ISyncable* entity);
            // Posts to pending queue — safe to call from any thread/enqueue context.
            void enqueue_syncable_registration(Engine::ISyncable* entity);
            void cleanup_remote_bodies();

            // Clears sync_sleeping on every locally-authoritative syncable (without
            // touching sync_last_pos/rot), forcing one real resend before it re-sleeps.
            // Called for the server's own models and mirrored to clients via
            // "_wake_sync" whenever a new peer joins, so late-joiners get a
            // correcting packet for every entity within one tick.
            void wake_all_syncables();

            #if defined(VSDK_Client)
            // Buffers a shape sync for a net_id not yet registered; poll() applies
            // it via Engine::Network::apply_shape() once it registers.
            void defer_shape_sync(uint32_t net_id, const godot::String& shape_type, const godot::Array& params);

            // Buffers a transform sync for a net_id not yet registered; poll() applies
            // it via ISyncable::apply_sync() once it registers. See pending_transform_syncs.
            void defer_transform_sync(uint32_t net_id, const godot::Vector3& pos, const godot::Vector3& rot, const godot::Vector3& vel);

            // Called from Engine::Network::_sync_rate with the server's real
            // physics_tick_rate/sync_rate — keeps client-side sync_interval and
            // every ISyncable's interp_step from staying at compile-time defaults.
            void apply_sync_config(int rate, float buffer_delay_max, float jitter_margin, float snap_threshold);
            #endif


            // State //
            bool is_connected() const;
            bool is_connecting() const;
            int  get_peer_id() const;

            // ENet peer stats (ms / ratio). Returns -1 if peer is unknown or not connected.
            // Backed by ENetPacketPeer::get_statistic — zero extra traffic.
            double get_peer_rtt(int peer_id) const;            // mean RTT (PEER_ROUND_TRIP_TIME)
            double get_peer_last_rtt(int peer_id) const;       // last sample (PEER_LAST_ROUND_TRIP_TIME)
            double get_peer_packet_loss(int peer_id) const;    // loss ratio (PEER_PACKET_LOSS)

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
            const Engine::ISyncable::SyncConfig& get_sync_config() const { return Engine::ISyncable::sync_config; }
            std::string get_server_ip() const;
            #endif


            // Shared RPC (channel 0, reliable) //
            bool send(const Tool::Stack& stack, int peerID = 0);
            bool broadcast(const Tool::Stack& stack);
            bool send_to_server(const Tool::Stack& stack);

            // Sync transport — routed through Godot's RPC layer.
            bool broadcast_sync(const godot::PackedByteArray& data);      // server -> all clients (unreliable)
            bool send_sync_to_server(const godot::PackedByteArray& data);  // client -> server (unreliable)

            // Inbound dispatch — called by Engine::Network RPC handlers.
            // Works on any ISyncable type via the net_id registry.
            void dispatch_sync_batch(const godot::PackedByteArray& data, bool is_state_dump);
            void dispatch_client_sync(const godot::PackedByteArray& data, int sender_id);


            #if !defined(VSDK_Client)
            // Sends current transform of all registered syncables to a late-joining peer.
            void send_full_state_to_peer(int peer_id);
            #endif
            // O(1) lookup by net_id — available on both client and server builds.
            Engine::ISyncable* find_syncable(uint32_t net_id);

            // Expose the Engine::Network node so Model can call rpc() on it.
            Engine::Network* get_node() const { return node; }

            void poll(double delta = 0.0);

            // Fixed-timestep counterpart to poll() — called once per physics
            // tick from Core::_physics_process. Owns the actual "sample this
            // peer's authoritative transforms / decide who's moved / encode
            // + broadcast or relay a sync packet" work, which poll() used to
            // do from the variable render tick. Splitting it out means every
            // sample lines up with a real simulation step instead of an
            // arbitrary render frame — see the .cpp for the full story on why
            // that mismatch was producing visible jitter/overshoot on
            // interpolated remote bodies. poll() keeps everything else
            // (reconnect/handshake, pending-registration flush, buffered
            // shape/transform/reparent replay) on the render tick, since
            // those are about responsiveness, not physics timing.
            void sync_tick(double delta);
    };
}
