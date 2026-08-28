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
        public:
            // Delta compression constants //
            static constexpr int   SYNC_PACKET_MAX     = 42;     // max bytes per delta entry
            static constexpr float DELTA_POS_THRESHOLD = 0.001f; // metres
            static constexpr float DELTA_ROT_THRESHOLD = 0.05f;  // degrees
            static constexpr float DELTA_VEL_THRESHOLD = 0.01f;  // units/sec

            // Snapshot buffer constants //
            static constexpr int   SNAPSHOT_COUNT   = 8;
            static constexpr float BUFFER_DELAY     = 0.1f;   // 100ms render lag
            static constexpr float SNAP_THRESHOLD   = 5.0f;   // units — teleport if gap exceeds this
            static constexpr float VEL_THRESHOLD    = 0.05f;  // units/sec — "moving" cutoff
            static constexpr float BUFFER_DELAY_MIN = 0.05f;  // 50ms floor
            static constexpr float BUFFER_DELAY_MAX = 0.25f;  // 250ms ceiling
            static constexpr float JITTER_MARGIN    = 2.0f;   // stddev multiplier
            static constexpr int   JITTER_WINDOW    = 8;      // samples

            // Type //
            // Identifies the concrete type for spawn/destroy RPCs so the
            // network layer can instantiate the right class on receivers.
            enum class SyncType : uint8_t {
                Model       = 0,
                PhysicsBody = 1
            };
        private:
            friend class Model;
            inline static uint32_t next_net_id = 1;
            static constexpr uint16_t MASK_PX = 1 << 0;
            static constexpr uint16_t MASK_PY = 1 << 1;
            static constexpr uint16_t MASK_PZ = 1 << 2;
            static constexpr uint16_t MASK_RX = 1 << 3;
            static constexpr uint16_t MASK_RY = 1 << 4;
            static constexpr uint16_t MASK_RZ = 1 << 5;
            static constexpr uint16_t MASK_VX = 1 << 6;
            static constexpr uint16_t MASK_VY = 1 << 7;
            static constexpr uint16_t MASK_VZ = 1 << 8;

            static void write_u32(godot::PackedByteArray& buf, int off, uint32_t v);
            static void write_u16(godot::PackedByteArray& buf, int off, uint16_t v);
            static void write_f32(godot::PackedByteArray& buf, int off, float v);
            static float read_f32 (const godot::PackedByteArray& buf, int off);
            static uint16_t read_u16(const godot::PackedByteArray& buf, int off);
            static int encode_delta(godot::PackedByteArray& buf, int offset, uint32_t id, godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel);
            static int decode_delta(const godot::PackedByteArray& buf, int offset, int buf_size, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel);
        protected:
            int sync_authority = 1; // 1 = server, N = client N
            uint32_t net_id = 0;
            bool sync_registered = false;
            bool sync_sleeping = false;
            float sync_accum = 0.0f;
            godot::Vector3 sync_last_pos;
            godot::Vector3 sync_last_rot;
            godot::Vector3 sync_last_vel;

            struct Snapshot {
                godot::Vector3 pos;
                godot::Vector3 rot;
                godot::Vector3 vel;
                float time = -1.0f;
            };

            Snapshot snap_buf[SNAPSHOT_COUNT];
            int snap_head   = 0;
            int snap_count  = 0;
            float snap_clock  = 0.0f;
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

            // Call from apply_sync — handles snapshot write + jitter measurement.
            void sync_push_snapshot(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel);
            // Call from on_sync_process — drives snapshot buffer interpolation.
            void interp_process(double delta, godot::Vector3& out_pos, godot::Vector3& out_rot);

            ISyncable() = default;
            virtual ~ISyncable() = default;
        public:
            // ── Public packet helpers ───────────────────────────────────────
            static uint32_t read_u32_public(const godot::PackedByteArray& buf, int offset) {
                return (uint8_t)buf[offset]
                     | ((uint8_t)buf[offset+1] << 8)
                     | ((uint8_t)buf[offset+2] << 16)
                     | ((uint8_t)buf[offset+3] << 24);
            }

            static int encode_delta_public(godot::PackedByteArray& buf, int offset, uint32_t id, godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel);
            static int decode_delta_public(const godot::PackedByteArray& buf, int offset, int buf_size, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel);
            int parse_sync_packet_at(const godot::PackedByteArray& buf, int offset, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel);

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
    };
}
