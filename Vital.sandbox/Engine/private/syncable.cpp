/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: syncable.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: ISyncable Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/syncable.h>
#include <Vital.sandbox/Manager/public/network.h>


///////////////////////////////
// Vital: Engine: ISyncable //
///////////////////////////////

namespace Vital::Engine {
    void ISyncable::Internal::write_u32(godot::PackedByteArray& buffer, int offset, uint32_t value) {
        buffer[offset]   =  value        & 0xFF;
        buffer[offset+1] = (value >>  8) & 0xFF;
        buffer[offset+2] = (value >> 16) & 0xFF;
        buffer[offset+3] = (value >> 24) & 0xFF;
    }

    void ISyncable::Internal::write_u16(godot::PackedByteArray& buffer, int offset, uint16_t value) {
        buffer[offset]   =  value       & 0xFF;
        buffer[offset+1] = (value >> 8) & 0xFF;
    }

    void ISyncable::Internal::write_f32(godot::PackedByteArray& buffer, int offset, float value) {
        uint32_t raw; std::memcpy(&raw, &value, 4); write_u32(buffer, offset, raw);
    }

    float ISyncable::Internal::read_f32(const godot::PackedByteArray& buffer, int offset) {
        uint32_t raw = (uint8_t)buffer[offset]
                     | ((uint8_t)buffer[offset+1] << 8)
                     | ((uint8_t)buffer[offset+2] << 16)
                     | ((uint8_t)buffer[offset+3] << 24);
        float value; std::memcpy(&value, &raw, 4); return value;
    }

    uint16_t ISyncable::Internal::read_u16(const godot::PackedByteArray& buffer, int offset) {
        return (uint8_t)buffer[offset] | ((uint8_t)buffer[offset+1] << 8);
    }

    uint32_t ISyncable::Internal::read_u32(const godot::PackedByteArray& buffer, int offset) {
        return (uint8_t)buffer[offset]
             | ((uint8_t)buffer[offset+1] << 8)
             | ((uint8_t)buffer[offset+2] << 16)
             | ((uint8_t)buffer[offset+3] << 24);
    }

    int ISyncable::Internal::encode_delta(godot::PackedByteArray& buffer, int offset, uint32_t id, godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel) {
        uint16_t mask = 0;
        if (std::abs(pos.x - last_pos.x) > DELTA_POS_THRESHOLD) mask |= MASK_PX;
        if (std::abs(pos.y - last_pos.y) > DELTA_POS_THRESHOLD) mask |= MASK_PY;
        if (std::abs(pos.z - last_pos.z) > DELTA_POS_THRESHOLD) mask |= MASK_PZ;
        if (std::abs(rot.x - last_rot.x) > DELTA_ROT_THRESHOLD) mask |= MASK_RX;
        if (std::abs(rot.y - last_rot.y) > DELTA_ROT_THRESHOLD) mask |= MASK_RY;
        if (std::abs(rot.z - last_rot.z) > DELTA_ROT_THRESHOLD) mask |= MASK_RZ;
        if (std::abs(vel.x - last_vel.x) > DELTA_VEL_THRESHOLD) mask |= MASK_VX;
        if (std::abs(vel.y - last_vel.y) > DELTA_VEL_THRESHOLD) mask |= MASK_VY;
        if (std::abs(vel.z - last_vel.z) > DELTA_VEL_THRESHOLD) mask |= MASK_VZ;

        write_u32(buffer, offset, id);
        write_u16(buffer, offset + 4, mask);
        int cursor = offset + 6;
        auto maybe_write = [&](bool bit, float val, float& last) {
            if (!bit) return;
            write_f32(buffer, cursor, val);
            last   = val;
            cursor += 4;
        };

        maybe_write(mask & MASK_PX, pos.x, last_pos.x);
        maybe_write(mask & MASK_PY, pos.y, last_pos.y);
        maybe_write(mask & MASK_PZ, pos.z, last_pos.z);
        maybe_write(mask & MASK_RX, rot.x, last_rot.x);
        maybe_write(mask & MASK_RY, rot.y, last_rot.y);
        maybe_write(mask & MASK_RZ, rot.z, last_rot.z);
        maybe_write(mask & MASK_VX, vel.x, last_vel.x);
        maybe_write(mask & MASK_VY, vel.y, last_vel.y);
        maybe_write(mask & MASK_VZ, vel.z, last_vel.z);
        return cursor - offset;
    }

    int ISyncable::Internal::decode_delta(const godot::PackedByteArray& buffer, int offset, int buf_size, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel) {
        if (offset + 6 > buf_size) return -1;

        out_id = read_u32(buffer, offset);
        uint16_t mask = read_u16(buffer, offset + 4);
        int cursor = offset + 6;
        out_pos = last_pos;
        out_rot = last_rot;
        out_vel = last_vel;

        auto maybe_read = [&](bool bit, float& out, float& last) -> bool {
            if (!bit) return true;
            if (cursor + 4 > buf_size) return false;
            out = read_f32(buffer, cursor);
            last = out;
            cursor += 4;
            return true;
        };

        if (!maybe_read(mask & MASK_PX, out_pos.x, last_pos.x)) return -1;
        if (!maybe_read(mask & MASK_PY, out_pos.y, last_pos.y)) return -1;
        if (!maybe_read(mask & MASK_PZ, out_pos.z, last_pos.z)) return -1;
        if (!maybe_read(mask & MASK_RX, out_rot.x, last_rot.x)) return -1;
        if (!maybe_read(mask & MASK_RY, out_rot.y, last_rot.y)) return -1;
        if (!maybe_read(mask & MASK_RZ, out_rot.z, last_rot.z)) return -1;
        if (!maybe_read(mask & MASK_VX, out_vel.x, last_vel.x)) return -1;
        if (!maybe_read(mask & MASK_VY, out_vel.y, last_vel.y)) return -1;
        if (!maybe_read(mask & MASK_VZ, out_vel.z, last_vel.z)) return -1;
        return cursor - offset;
    }
}

namespace Vital::Engine {
    // Misc //
    int ISyncable::encode_delta(godot::PackedByteArray& buffer, int offset, uint32_t id, godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel) {
        return Internal::encode_delta(buffer, offset, id, pos, rot, vel, last_pos, last_rot, last_vel);
    }

    int ISyncable::decode_delta(const godot::PackedByteArray& buffer, int offset, int buf_size, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel) {
        return Internal::decode_delta(buffer, offset, buf_size, out_id, out_pos, out_rot, out_vel, last_pos, last_rot, last_vel);
    }

    int ISyncable::parse_sync_packet_at(const godot::PackedByteArray& buffer, int offset, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel) {
        return Internal::decode_delta(buffer, offset, (int)buffer.size(), out_id, out_pos, out_rot, out_vel, delta_last_pos, delta_last_rot, delta_last_vel);
    }

    void ISyncable::reset_sync_state() {
        snap_head = 0;
        snap_count = 0;
        snap_clock = 0.0f;
        interp_ready = false;
        jitter_last_arrival = -1.0f;
        jitter_idx = 0;
        jitter_count = 0;
        adaptive_delay = BUFFER_DELAY;
        for (int i = 0; i < JITTER_WINDOW; i++) jitter_intervals[i] = 0.0f;
    }

    #if !defined(VSDK_Client)
    // STATE_DUMP_MAGIC from network.cpp — duplicated here to avoid a
    // circular header dependency.  Keep in sync with that value.
    static constexpr uint32_t FORCE_SYNC_MAGIC = 0x56535354u; // 'VSST'

    void ISyncable::flush_pending_force_transform() {
        if (!pending_force_transform.has_value()) return;
        if (sync_authority <= 1) { pending_force_transform.reset(); return; }
        auto* net_node = Manager::Network::get_singleton()->get_node();
        if (net_node)
            net_node->rpc_id(sync_authority, "_force_transform",
                (int)net_id,
                pending_force_transform->pos,
                pending_force_transform->rot);
        pending_force_transform.reset();
    }

    void ISyncable::force_transform_broadcast() {
        godot::Vector3 cur_pos = get_sync_position();
        godot::Vector3 cur_rot = get_sync_rotation();

        if (net_id == 0) {
            // Not yet registered — stash for flush_pending_force_transform(),
            // which the deferred registration lambda calls after _spawn_entity.
            // The unreliable broadcast is skipped (no net_id to encode), but
            // _spawn_entity already carries get_sync_position()/get_sync_rotation()
            // fresh, so all non-owning clients get the correct position via that.
            pending_force_transform = PendingForceTransform{ cur_pos, cur_rot };
            return;
        }

        godot::Vector3 cur_vel = godot::Vector3(); // standstill after a teleport

        // Reseed the baseline so the comparison inside sync_tick sees no
        // delta on the next tick (entity appears still from here).
        // delta_last_* are the running state for ISyncable's encode_delta;
        // reset them too so the first incremental packet from this new origin
        // is encoded correctly (no stale previous-position residual).
        sync_last_pos  = cur_pos;
        sync_last_rot  = cur_rot;
        sync_last_vel  = cur_vel;
        delta_last_pos = cur_pos;
        delta_last_rot = cur_rot;
        delta_last_vel = godot::Vector3();
        sync_sleeping  = false;
        sync_accum     = 0.0f;

        // Build a minimal single-entity VSST batch and fire it.
        godot::PackedByteArray buf;
        buf.resize(8 + SYNC_PACKET_MAX);

        // Scratch accumulators for encode_delta (start zeroed so it
        // treats this as the first packet from a fresh baseline).
        godot::Vector3 enc_pos{}, enc_rot{}, enc_vel{};
        int written = encode_delta(buf, 8, net_id,
            cur_pos, cur_rot, cur_vel,
            enc_pos, enc_rot, enc_vel);
        if (written <= 0) return;

        buf.resize(8 + written);
        // Header: magic + payload length
        auto wu32 = [&](int off, uint32_t v) {
            buf[off]   =  v        & 0xFF;
            buf[off+1] = (v >>  8) & 0xFF;
            buf[off+2] = (v >> 16) & 0xFF;
            buf[off+3] = (v >> 24) & 0xFF;
        };
        wu32(0, FORCE_SYNC_MAGIC);
        wu32(4, (uint32_t)written);

        Manager::Network::get_singleton()->broadcast_sync(buf);

        // The unreliable broadcast above reaches every peer EXCEPT the owning
        // client, whose apply_sync() drops packets for entities it holds
        // authority over.  Send a reliable targeted RPC directly to that peer
        // so it also repositions the entity and reseeds its upload baseline —
        // preventing it from snapping the entity back on the next _sync_client.
        // Skip for server-authority entities (sync_authority == 1): nobody
        // needs a special override in that case because the server IS the
        // authority and already wrote the position above.
        if (sync_authority > 1) {
            auto* net_node = Manager::Network::get_singleton()->get_node();
            if (net_node)
                net_node->rpc_id(sync_authority, "_force_transform",
                    (int)net_id, cur_pos, cur_rot);
        }
    }
    #endif

    void ISyncable::sync_push_snapshot(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel) {
        if (!interp_ready) {
            snap_clock = BUFFER_DELAY;
            jitter_last_arrival = snap_clock;
            interp_ready = true;
        }
        else {
            float interval = snap_clock - jitter_last_arrival;
            if (interval > 0.0f) {
                jitter_intervals[jitter_idx] = interval;
                jitter_idx = (jitter_idx + 1) % JITTER_WINDOW;
                if (jitter_count < JITTER_WINDOW) jitter_count++;
                float mean = 0.0f;
                for (int i = 0; i < jitter_count; i++) mean += jitter_intervals[i];
                mean /= (float)jitter_count;
                float variance = 0.0f;
                for (int i = 0; i < jitter_count; i++) {
                    float d = jitter_intervals[i] - mean;
                    variance += d * d;
                }
                float stddev = (jitter_count > 1) ? std::sqrt(variance / (float)(jitter_count - 1)) : 0.0f;
                // Target: one interp_step (one packet interval) + jitter headroom.
                // Clamp to [BUFFER_DELAY_MIN, BUFFER_DELAY_MAX] so we always keep at
                // least ~2 packet intervals of buffer ahead of the render clock, even
                // on a near-zero-jitter connection. A 1-packet floor (interp_step)
                // sounds tighter/more responsive, but it means the renderer is
                // constantly running out of a real "after" snapshot and falling into
                // interp_process's velocity-based extrapolation branch — which looks
                // fine for constant-velocity motion but visibly overshoots then snaps
                // back on every direction change, since it assumes velocity stays
                // constant. Keeping a real bracketing snapshot on hand avoids that.
                float target = std::clamp(interp_step + sync_config.jitter_margin * stddev, BUFFER_DELAY_MIN, sync_config.buffer_delay_max);
                // Faster EMA: 0.8 old + 0.2 new — responds to network changes in ~5 packets
                // instead of the old 0.95/0.05 which took ~20 packets to converge.
                adaptive_delay = adaptive_delay * 0.8f + target * 0.2f;
            }
            jitter_last_arrival = snap_clock;
        }

        Snapshot& slot = snap_buf[snap_head];
        slot.pos = pos;
        slot.rot = rot;
        slot.vel = vel;
        slot.time = snap_clock - adaptive_delay;
        snap_head = (snap_head + 1) % SNAPSHOT_COUNT;
        if (snap_count < SNAPSHOT_COUNT) snap_count++;
    }

    void ISyncable::interp_process(double delta, godot::Vector3& out_pos, godot::Vector3& out_rot) {
        if (!interp_ready || snap_count == 0) return;

        snap_clock += static_cast<float>(delta);
        float render_time = snap_clock - adaptive_delay;
        const Snapshot* before = nullptr;
        const Snapshot* after  = nullptr;
        for (int i = 0; i < snap_count; i++) {
            int idx = (snap_head - snap_count + i + SNAPSHOT_COUNT) % SNAPSHOT_COUNT;
            const Snapshot& s = snap_buf[idx];
            if (s.time <= render_time) before = &s;
            else if (!after) after = &s;
        }

        if (!before && !after) return;
        if (!before) {
            out_pos = after -> pos;
            out_rot = after -> rot;
            return;
        }
        if (!after) {
            if (before -> vel.length() > VEL_THRESHOLD) {
                // Cap extrapolation to 1 interp step (was 2). We only reach this
                // branch on a genuine buffer underrun — the "after" snapshot we
                // normally keep buffered hasn't arrived yet — and a hard
                // direction reversal is the worst case for it: the real velocity
                // has already flipped but we're still projecting forward on the
                // old one. Halving the cap halves how far/long that guess can be
                // wrong before the next real snapshot corrects it.
                float cap = interp_step;
                float extra = std::min(render_time - before -> time, cap);
                // Ease the extrapolated contribution toward zero across the
                // window instead of holding it at full velocity throughout —
                // bounds worst-case overshoot on a reversal without needing to
                // predict the reversal (which is impossible from position/
                // velocity alone). Full weight at the start (t=0, where we're
                // most likely still correct), tapering to half weight by the
                // time we hit the cap.
                float t = cap > 0.0f ? std::clamp(extra / cap, 0.0f, 1.0f) : 0.0f;
                float ease = 1.0f - t * 0.5f;
                out_pos = before -> pos + before -> vel * extra * ease;
            }
            else out_pos = before -> pos;
            out_rot = before -> rot;
            return;
        }

        float span = after -> time - before -> time;
        if (span <= 0.0f) { out_pos = after -> pos; out_rot = after -> rot; return; }
        float t = std::clamp((render_time - before -> time) / span, 0.0f, 1.0f);
        if (before -> pos.distance_to(after -> pos) > sync_config.snap_threshold) {
            out_pos = after -> pos;
            out_rot = after -> rot;
        } 
        else {
            out_pos = before -> pos.lerp(after -> pos, t);
            // Slerp via quaternion to avoid Euler gimbal/wrap issues (e.g. 359->1 deg).
            static constexpr float DEG2RAD = 3.14159265358979323846f / 180.0f;
            static constexpr float RAD2DEG = 180.0f / 3.14159265358979323846f;
            godot::Quaternion q_before = godot::Basis::from_euler(before -> rot * DEG2RAD).get_quaternion();
            godot::Quaternion q_after  = godot::Basis::from_euler(after  -> rot * DEG2RAD).get_quaternion();
            godot::Quaternion q_interp = q_before.slerp(q_after, t);
            out_rot = godot::Basis(q_interp).get_euler() * RAD2DEG;
        }
    }

    #if !defined(VSDK_Client)
    void ISyncable::set_parent(godot::Node3D* parent_node) {
        auto core = Engine::Core::get_singleton();
        if (!core) return;

        godot::Node3D* self_node = get_sync_node();
        if (!self_node) return;

        core->when_parent_ready(self_node, parent_node,
            [](godot::Node3D* self_n, godot::Node* parent) {
                if (auto* syncable = dynamic_cast<ISyncable*>(self_n))
                    syncable->apply_parent(parent);
            });
    }

    void ISyncable::apply_parent(godot::Node* parent_node) {
        auto core = Engine::Core::get_singleton();
        if (!core) return;

        godot::Node3D* self_node = get_sync_node();
        if (!self_node) return;

        uint32_t parent_net_id = 0;
        godot::Node* target     = core;

        if (parent_node && parent_node != target) {
            if (auto* syncable = dynamic_cast<ISyncable*>(parent_node)) {
                parent_net_id = syncable->get_net_id();
                target = parent_node;
            }
        }

        if (self_node->get_parent() != target) {
            self_node->reparent(target, true);  // keep_global_transform = true
        }

        // Switch sync space to local (or back to global when detaching).
        // Re-seed delta baselines from the new coordinate space so the very next
        // encode_delta produces a clean full packet rather than a stale diff.
        sync_parent_net_id = parent_net_id;
        sync_last_pos      = get_sync_position();  // now returns local if parented
        sync_last_rot      = get_sync_rotation();
        sync_last_vel      = godot::Vector3();
        sync_sleeping      = false;

        // Defer the _reparent_entity RPC to the next enqueue flush so it always
        // arrives on clients AFTER the _spawn_entity RPCs for both this entity
        // and its parent — which are themselves enqueued from Model::create /
        // Physics_Body::setup_create.  Firing it inline here would race those
        // and the client would silently drop the reparent (find_syncable returns
        // null because the entity isn't registered yet).
        uint32_t captured_net_id    = net_id;
        uint32_t captured_parent_id = parent_net_id;
        core->enqueue([captured_net_id, captured_parent_id]() {
            auto* net_node = Manager::Network::get_singleton()->get_node();
            if (net_node)
                net_node->rpc("_reparent_entity",
                    (int)captured_net_id, (int)captured_parent_id);
            godot::UtilityFunctions::print(
                "ISyncable::apply_parent net_id=", captured_net_id,
                " -> parent_net_id=", captured_parent_id);
        });
    }

    uint32_t ISyncable::get_parent_net_id() const {
        auto* self_node = const_cast<ISyncable*>(this)->get_sync_node();
        if (!self_node || !self_node->is_inside_tree()) return 0;
        auto* syncable = dynamic_cast<const ISyncable*>(self_node->get_parent());
        return syncable ? syncable->get_net_id() : 0;
    }
    #endif
}
