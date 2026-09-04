/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: syncable.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: ISyncable Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>


///////////////////////////////
// Vital: Engine: ISyncable //
///////////////////////////////

namespace Vital::Manager { class Network; }
namespace Vital::Engine  { class Network; }
namespace Vital::Engine {
    class ISyncable {
        friend class Manager::Network;
        friend class Network;
        friend class Model;
        public:
            // Delta compression constants //
            static constexpr int   SYNC_PACKET_MAX     = 42;     // max bytes per delta entry
            static constexpr float DELTA_POS_THRESHOLD = 0.001f; // metres
            static constexpr float DELTA_ROT_THRESHOLD = 0.05f;  // degrees
            static constexpr float DELTA_VEL_THRESHOLD = 0.01f;  // units/sec

            // Snapshot buffer constants //
            static constexpr int   SYNC_RATE        = 60;    // default sync rate in Hz
            static constexpr int   SNAPSHOT_COUNT   = 32;    // ~530ms of history at 60Hz —
                                                              // headroom above BUFFER_DELAY_MAX
                                                              // so a big adaptive buffer still
                                                              // has real snapshots behind it.

            // Compile-time defaults — the adaptive system converges away from these
            // within ~5 packets (0.2 EMA). Server operators override the three
            // tuneable values below via config.yaml (sync section); see Config::Server.
            static constexpr float BUFFER_DELAY     = 0.033f; // seed — 2 packets at 60Hz; adapts up fast
            static constexpr float SNAP_THRESHOLD   = 5.0f;   // units — teleport if gap exceeds this
            static constexpr float VEL_THRESHOLD    = 0.05f;  // units/sec — "moving" cutoff
            static constexpr float BUFFER_DELAY_MIN = 0.033f; // floor — keeps real bracketing snapshot;
                                                               // must stay >= interp_step (1/sync_rate)
                                                               // or the renderer falls into the
                                                               // extrapolation branch on every tick.
            static constexpr float BUFFER_DELAY_MAX = 0.30f;  // default ceiling (300ms) — overridable
            static constexpr float JITTER_MARGIN    = 1.5f;   // default stddev multiplier — overridable
            static constexpr int   JITTER_WINDOW    = 16;     // more samples for stable estimate

            struct SyncConfig {
                int   rate             = SYNC_RATE;
                float buffer_delay_max = BUFFER_DELAY_MAX;
                float jitter_margin    = JITTER_MARGIN;
                float snap_threshold   = SNAP_THRESHOLD;
            };
            // Written directly by Manager::Network on host()/connect (avoids a circular
            // include between Engine/syncable.cpp and Manager/network.h). Defaults to
            // the compile-time values above, so it's always safe to read.
            static inline SyncConfig sync_config;


            // Type //
            // Identifies the concrete type for spawn/destroy RPCs so the
            // network layer can instantiate the right class on receivers.
            enum class SyncType : uint8_t {
                Model       = 0,
                PhysicsBody = 1
            };
        private:
            struct Internal {
                static constexpr uint16_t MASK_PX = 1 << 0;
                static constexpr uint16_t MASK_PY = 1 << 1;
                static constexpr uint16_t MASK_PZ = 1 << 2;
                static constexpr uint16_t MASK_RX = 1 << 3;
                static constexpr uint16_t MASK_RY = 1 << 4;
                static constexpr uint16_t MASK_RZ = 1 << 5;
                static constexpr uint16_t MASK_VX = 1 << 6;
                static constexpr uint16_t MASK_VY = 1 << 7;
                static constexpr uint16_t MASK_VZ = 1 << 8;

                // Helpers //
                static void write_u32(godot::PackedByteArray& buffer, int offset, uint32_t value);
                static void write_u16(godot::PackedByteArray& buffer, int offset, uint16_t value);
                static void write_f32(godot::PackedByteArray& buffer, int offset, float value);
                static float read_f32(const godot::PackedByteArray& buffer, int offset);
                static uint16_t read_u16(const godot::PackedByteArray& buffer, int offset);
                static uint32_t read_u32(const godot::PackedByteArray& buffer, int offset);
                static int encode_delta(godot::PackedByteArray& buffer, int offset, uint32_t id, godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel);
                static int decode_delta(const godot::PackedByteArray& buffer, int offset, int buf_size, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel);
            };
        protected:
            inline static uint32_t next_net_id = 1;
            int sync_authority = 1; // 1 = server, N = client N
            uint32_t net_id = 0;
            bool sync_registered = false;
            bool sync_sleeping = false;
            float sync_accum = 0.0f;
            godot::Vector3 sync_last_pos;
            godot::Vector3 sync_last_rot;
            godot::Vector3 sync_last_vel;

            // When non-zero, this entity is parented under another synced entity.
            // All transforms sent and received are LOCAL (relative to parent) rather
            // than global.  The child goes to sleep on its own when its local offset
            // is stable — even if the parent is carrying it around — so we never pay
            // for positions that haven't changed in the parent's frame.
            // Set by Model::set_parent() on the server and by _reparent_entity on
            // clients.  Reset to 0 on detach.
            uint32_t sync_parent_net_id = 0;

            struct Snapshot {
                godot::Vector3 pos;
                godot::Vector3 rot;
                godot::Vector3 vel;
                float time = -1.0f;
            };

            Snapshot snap_buf[SNAPSHOT_COUNT];
            int snap_head = 0;
            int snap_count = 0;
            float snap_clock = 0.0f;
            float interp_step = 1.0f / 20.0f;
            bool interp_ready = false;
            float jitter_last_arrival = -1.0f;
            float jitter_intervals[JITTER_WINDOW] = {};
            int jitter_idx = 0;
            int jitter_count = 0;
            float adaptive_delay = BUFFER_DELAY;
            godot::Vector3 delta_last_pos;
            godot::Vector3 delta_last_rot;
            godot::Vector3 delta_last_vel;


            // Instantiators //
            ISyncable() = default;
            virtual ~ISyncable() = default;


            // Helpers //
            void sync_push_snapshot(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel);
            void interp_process(double delta, godot::Vector3& out_pos, godot::Vector3& out_rot);

            // Bridges this mixin interface to the concrete Node3D each synced
            // type is layered onto — Model IS-A Node3D directly; Physics_Body<Base>
            // is layered onto a Base that IS-A Node3D (RigidBody3D, StaticBody3D,
            // CharacterBody3D, ...). set_parent()/apply_parent() below need to
            // touch the actual scene graph (reparent, is_inside_tree, get_parent)
            // without knowing the concrete type, hence this hook. Every subclass
            // implements it as a trivial `return this;`.
            virtual godot::Node3D* get_sync_node() = 0;

            #if !defined(VSDK_Client)
            // Actual reparent + sync-baseline-reset + _reparent_entity broadcast,
            // factored out of set_parent() so it can be handed to
            // Core::when_parent_ready() as a plain callback and only ever run
            // once both this entity and the requested parent are confirmed to
            // be inside the scene tree. `parent_node` is nullable (detach).
            void apply_parent(godot::Node* parent_node);
            #endif
        public:
            // Misc //
            static uint32_t read_u32(const godot::PackedByteArray& buffer, int offset) {
                return Internal::read_u32(buffer, offset);
            }

            static int encode_delta(godot::PackedByteArray& buffer, int offset, uint32_t id, godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel);
            static int decode_delta(const godot::PackedByteArray& buffer, int offset, int buf_size, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel);
            int parse_sync_packet_at(const godot::PackedByteArray& buffer, int offset, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel);
            virtual SyncType get_sync_type() const = 0;
            virtual bool is_sync_active() const = 0;
            void set_sync_authority(int peer_id) {
                sync_authority = peer_id;
                reset_sync_state();
            }

            virtual void destroy_sync() = 0;
            virtual std::string get_sync_name() const { return ""; }
            virtual uint32_t get_net_id() const { return net_id; }
            virtual int get_sync_authority() const { return sync_authority; }
            virtual godot::Vector3 get_sync_position() const = 0;
            virtual godot::Vector3 get_sync_rotation() const = 0;
            virtual void apply_sync(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel) = 0;
            virtual void on_sync_process(double delta) = 0;
            virtual void reset_sync_state();

            #if !defined(VSDK_Client)
            // set_parent(parent_node) — reparent this server-authoritative synced
            //   entity under another synced entity (Model, Physics_Body, ...) on
            //   the server scene tree and broadcast _reparent_entity to all
            //   current clients. Pass nullptr to detach back to Core root
            //   (sync_parent_net_id == 0). ONLY call from the server VM; the API
            //   layer enforces this.
            //   Shared across every ISyncable type — Model, Physics_Body, and
            //   anything added later — so none of them have to reimplement this
            //   (or, worse, fall back to a raw, un-synced reparent() that never
            //   tells clients).
            // get_parent_net_id() — returns the net_id of the current sync
            //   parent, or 0 when parented directly to Core.
            void     set_parent(godot::Node3D* parent_node);
            uint32_t get_parent_net_id() const;
            #endif
    };
}
