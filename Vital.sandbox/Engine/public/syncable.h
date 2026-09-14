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

// TODO: Improve
namespace Vital::Manager {
    class Network;
}

namespace Vital::Engine {
    class Network;
    class ISyncable {
        friend class Manager::Network;
        friend class Network;
        friend class Model;
        public:
            static constexpr int   SYNC_PACKET_MAX     = 54;     // max bytes per delta entry (pos+rot+vel+scale)
            static constexpr float DELTA_POS_THRESHOLD = 0.001f; // metres
            static constexpr float DELTA_ROT_THRESHOLD = 0.05f;  // degrees
            static constexpr float DELTA_VEL_THRESHOLD = 0.01f;  // units/sec
            static constexpr int   SYNC_RATE           = 60;     // default sync rate in Hz
            static constexpr int   SNAPSHOT_COUNT      = 32;     // ~530ms of history at 60Hz — headroom above BUFFER_DELAY_MAX so a big adaptive buffer still has real snapshots behind it.
            static constexpr float BUFFER_DELAY        = 0.033f; // seed — 2 packets at 60Hz; adapts up fast
            static constexpr float SNAP_THRESHOLD      = 5.0f;   // units — teleport if gap exceeds this
            static constexpr float VEL_THRESHOLD       = 0.05f;  // units/sec — "moving" cutoff
            static constexpr float BUFFER_DELAY_MIN    = 0.033f; // floor — keeps real bracketing snapshot; must stay >= interp_step (1/sync_rate) or the renderer falls into the extrapolation branch on every tick.
            static constexpr float BUFFER_DELAY_MAX    = 0.30f;  // default ceiling (300ms) — overridable
            static constexpr float JITTER_MARGIN       = 1.5f;   // default stddev multiplier — overridable
            static constexpr int   JITTER_WINDOW       = 16;     // more samples for stable estimate

            enum class SyncType : uint8_t {
                Model = 0,
                PhysicsBody = 1
            };

            struct SyncConfig {
                int rate = SYNC_RATE;
                float buffer_delay_max = BUFFER_DELAY_MAX;
                float jitter_margin = JITTER_MARGIN;
                float snap_threshold = SNAP_THRESHOLD;
            };
            static inline SyncConfig sync_config;

            #if !defined(VSDK_Client)
            struct PendingForceTransform {
                godot::Vector3 pos;
                godot::Vector3 rot;
                godot::Vector3 scale = godot::Vector3(1, 1, 1);
            };
            #endif
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
                static constexpr uint16_t MASK_SX = 1 << 9;
                static constexpr uint16_t MASK_SY = 1 << 10;
                static constexpr uint16_t MASK_SZ = 1 << 11;
                static constexpr float DELTA_SCALE_THRESHOLD = 0.001f;


                // Helpers //
                static void write_u32(godot::PackedByteArray& buffer, int offset, uint32_t value);
                static void write_u16(godot::PackedByteArray& buffer, int offset, uint16_t value);
                static void write_f32(godot::PackedByteArray& buffer, int offset, float value);
                static float read_f32(const godot::PackedByteArray& buffer, int offset);
                static uint16_t read_u16(const godot::PackedByteArray& buffer, int offset);
                static uint32_t read_u32(const godot::PackedByteArray& buffer, int offset);
                static int encode_delta(godot::PackedByteArray& buffer, int offset, uint32_t id, godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3 scale, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel, godot::Vector3& last_scale);
                static int decode_delta(const godot::PackedByteArray& buffer, int offset, int buf_size, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel, godot::Vector3& out_scale, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel, godot::Vector3& last_scale);
            };
        protected:
            inline static uint32_t next_net_id = 1;
            int sync_authority = 1; // 1 = server, N = client N
            uint32_t net_id = 0;
            uint32_t sync_parent_net_id = 0;
            bool sync_registered = false;
            bool sync_sleeping = false;
            float sync_accum = 0.0f;
            godot::Vector3 sync_last_pos;
            godot::Vector3 sync_last_rot;
            godot::Vector3 sync_last_vel;
            godot::Vector3 sync_last_scale = godot::Vector3(1, 1, 1);

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
            godot::Vector3 delta_last_scale = godot::Vector3(1, 1, 1);

            #if !defined(VSDK_Client)
            std::optional<PendingForceTransform> pending_force_transform;
            #endif


            // Instantiators //
            ISyncable() = default;
            virtual ~ISyncable() = default;


            // Helpers //
            void sync_push_snapshot(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel);
            void interp_process(double delta, godot::Vector3& out_pos, godot::Vector3& out_rot);
            virtual godot::Node3D* get_sync_node() = 0;
            #if !defined(VSDK_Client)
            void apply_parent(godot::Node* parent_node);
            #endif
        public:
            // Managers //
            virtual void destroy_sync() = 0;
            virtual void apply_sync(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3 scale) = 0;
            virtual void on_sync_process(double delta) = 0;
            virtual void reset_sync_state();
            #if !defined(VSDK_Client)
            void force_transform_broadcast();
            void flush_pending_force_transform();
            void set_parent(godot::Node3D* parent_node);
            #endif


            // Checkers //
            virtual bool is_sync_active() const = 0;
            bool is_replicated() const { return net_id != 0; }


            // Getters //
            virtual SyncType get_sync_type() const = 0;
            virtual std::string get_sync_name() const { return ""; }
            virtual uint32_t get_net_id() const { return net_id; }
            virtual int get_sync_authority() const { return sync_authority; }
            virtual godot::Vector3 get_sync_position() const = 0;
            virtual godot::Vector3 get_sync_rotation() const = 0;
            virtual godot::Vector3 get_sync_scale() const = 0;
            uint32_t get_sync_parent_net_id() const { return sync_parent_net_id; }
            #if !defined(VSDK_Client)
            uint32_t get_parent_net_id() const;
            #endif


            // Setters //
            void set_sync_authority(int peer_id) { sync_authority = peer_id; reset_sync_state(); }
            void set_sync_parent_net_id(uint32_t id) { sync_parent_net_id = id; }


            // Misc //
            static uint32_t read_u32(const godot::PackedByteArray& buffer, int offset) { return Internal::read_u32(buffer, offset); }
            static int encode_delta(godot::PackedByteArray& buffer, int offset, uint32_t id, godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3 scale, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel, godot::Vector3& last_scale);
            static int decode_delta(const godot::PackedByteArray& buffer, int offset, int buf_size, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel, godot::Vector3& out_scale, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel, godot::Vector3& last_scale);
            int parse_sync_packet_at(const godot::PackedByteArray& buffer, int offset, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel, godot::Vector3& out_scale);
    };
}
