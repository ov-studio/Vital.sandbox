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

            // Model sync registry — every live Model registers here so
            // poll() can drive sync_tick() without touching the scene tree.
            std::vector<Engine::ISyncable*> sync_models;
            std::mutex sync_models_mutex;

            // Persistent net_id -> Model* map — updated on register/unregister,
            // used by dispatch_client_sync and dispatch_sync_batch for O(1) lookup
            // without rebuilding a temporary map every packet.
            std::unordered_map<uint32_t, Engine::ISyncable*> sync_id_map;

            // Pending registration queue — models added from enqueue() callbacks
            // post to this instead of directly into sync_models, avoiding the
            // O(N) child-scan in poll().
            std::vector<Engine::ISyncable*> sync_pending;
            std::mutex sync_pending_mutex;

            #if defined(VSDK_Client)
            // Shape syncs that arrived (via _sync_shape RPC) before their parent
            // body finished local registration — the server can broadcast a shape
            // the same tick it creates the body, while the body's own spawn RPC
            // isn't sent until Rigid_Body::create's deferred registration runs a
            // frame later. Buffered here, keyed by net_id, and replayed by poll()
            // the moment that net_id shows up in sync_pending.
            std::unordered_map<uint32_t, std::pair<godot::String, godot::Array>> pending_shape_syncs;
            std::mutex pending_shape_mutex;

            // Same problem, for transform (position/rotation/velocity) packets.
            // dispatch_sync_batch's two reliable state-dump sends fire back-to-back
            // right after the spawn RPCs, both on the same channel — easily before
            // that frame's poll() has drained sync_pending, so every entry in the
            // dump can arrive for a net_id the client hasn't registered yet. Without
            // buffering, those entries were silently discarded (regardless of which
            // peer owns the body), leaving it at whatever transform _spawn_entity
            // defaulted it to (world origin) until a later live delta happened to
            // correct it — which only ever arrives if the body moves. Stores only
            // the latest decoded state per net_id; poll() applies and clears it the
            // moment that net_id registers.
            std::unordered_map<uint32_t, std::tuple<godot::Vector3, godot::Vector3, godot::Vector3>> pending_transform_syncs;
            std::mutex pending_transform_mutex;
            #endif

            // Per-frame dirty batch buffer reused across frames (avoids realloc).
            godot::PackedByteArray sync_batch_buf;

            // Sync interval in seconds — set from config on host(), read each poll().
            // Default 1/20 = 20 Hz. Configurable via network.sync_rate in config.yaml.
            float sync_interval = 1.0f / 20.0f;
            // Same value as an integer Hz — set alongside sync_interval in host().
            // Sent verbatim to each peer on connect via the "_sync_rate" RPC so
            // client builds (which never call host()) stop defaulting to 20 Hz.
            int   sync_rate_hz = 20;
            // Received from server via _sync_config RPC on connect.
            // Used by client-side interp in syncable.cpp.
            float sync_buffer_delay_max = Engine::ISyncable::BUFFER_DELAY_MAX;
            float sync_jitter_margin    = Engine::ISyncable::JITTER_MARGIN;
            float sync_snap_threshold   = Engine::ISyncable::SNAP_THRESHOLD;



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
            void register_syncable(Engine::ISyncable* entity);
            void unregister_syncable(Engine::ISyncable* entity);
            // Posts to pending queue — safe to call from any thread/enqueue context.
            void enqueue_syncable_registration(Engine::ISyncable* entity);
            void cleanup_remote_bodies();

            // Clears sync_sleeping on every locally-authoritative syncable (server
            // authority==1 models on a server build, or authority==my peer id on a
            // client build), without touching sync_last_pos/rot. The next poll()
            // tick then sees "not moved, not sleeping" and sends one real, current
            // transform update through the normal broadcast/relay path before
            // re-marking the body asleep — i.e. a one-off forced resend rather than
            // a permanent change in behaviour. Called on the server for its own
            // models, and mirrored to every connected client via the "_wake_sync"
            // RPC, whenever a new peer joins — so a late-joiner is guaranteed a
            // correcting packet for every entity within one tick of connecting,
            // including peer-authority bodies that were already asleep.
            void wake_all_syncables();

            #if defined(VSDK_Client)
            // Buffers a shape sync whose net_id isn't registered yet; poll() applies
            // it via Engine::Network::apply_shape() once that net_id registers.
            void defer_shape_sync(uint32_t net_id, const godot::String& shape_type, const godot::Array& params);

            // Buffers a transform sync (position/rotation/velocity) whose net_id
            // isn't registered yet; poll() applies it via ISyncable::apply_sync()
            // once that net_id registers. See pending_transform_syncs above.
            void defer_transform_sync(uint32_t net_id, const godot::Vector3& pos, const godot::Vector3& rot, const godot::Vector3& vel);

            // Called from Engine::Network::_sync_rate when the server tells us its
            // real physics_tick_rate/sync_rate. Without this, sync_interval (and
            // every ISyncable's interp_step derived from it) stays stuck at the
            // 20 Hz default above forever on a client build, regardless of what
            // config.yaml says — see network.cpp host() for the server-side half
            // of this that a pure client never runs.
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
            float get_sync_buffer_delay_max() const { return sync_buffer_delay_max; }
            float get_sync_jitter_margin()    const { return sync_jitter_margin; }
            float get_sync_snap_threshold()   const { return sync_snap_threshold; }
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
    };
}
