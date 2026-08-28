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


///////////////////////////////
// Vital: Engine: ISyncable //
///////////////////////////////

namespace Vital::Engine {
    void ISyncable::write_u32(godot::PackedByteArray& buf, int off, uint32_t v) {
        buf[off]   =  v        & 0xFF;
        buf[off+1] = (v >>  8) & 0xFF;
        buf[off+2] = (v >> 16) & 0xFF;
        buf[off+3] = (v >> 24) & 0xFF;
    }

    void ISyncable::write_u16(godot::PackedByteArray& buf, int off, uint16_t v) {
        buf[off]   =  v       & 0xFF;
        buf[off+1] = (v >> 8) & 0xFF;
    }

    void ISyncable::write_f32(godot::PackedByteArray& buf, int off, float v) {
        uint32_t raw; std::memcpy(&raw, &v, 4); write_u32(buf, off, raw);
    }

    float ISyncable::read_f32(const godot::PackedByteArray& buf, int off) {
        uint32_t raw = (uint8_t)buf[off]
                     | ((uint8_t)buf[off+1] << 8)
                     | ((uint8_t)buf[off+2] << 16)
                     | ((uint8_t)buf[off+3] << 24);
        float v; std::memcpy(&v, &raw, 4); return v;
    }

    uint16_t ISyncable::read_u16(const godot::PackedByteArray& buf, int off) {
        return (uint8_t)buf[off] | ((uint8_t)buf[off+1] << 8);
    }

    int ISyncable::encode_delta(godot::PackedByteArray& buf, int offset, uint32_t id, godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel) {
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

        write_u32(buf, offset, id);
        write_u16(buf, offset + 4, mask);
        int cursor = offset + 6;
        auto maybe_write = [&](bool bit, float val, float& last) {
            if (!bit) return;
            write_f32(buf, cursor, val);
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

    int ISyncable::decode_delta(const godot::PackedByteArray& buf, int offset, int buf_size, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel) {
        if (offset + 6 > buf_size) return -1;

        out_id = read_u32_public(buf, offset);
        uint16_t mask = read_u16(buf, offset + 4);
        int cursor = offset + 6;
        out_pos = last_pos;
        out_rot = last_rot;
        out_vel = last_vel;

        auto maybe_read = [&](bool bit, float& out, float& last) -> bool {
            if (!bit) return true;
            if (cursor + 4 > buf_size) return false;
            out = read_f32(buf, cursor);
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

    int ISyncable::encode_delta_public(godot::PackedByteArray& buf, int offset, uint32_t id, godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel) {
        return encode_delta(buf, offset, id, pos, rot, vel, last_pos, last_rot, last_vel);
    }

    int ISyncable::decode_delta_public(const godot::PackedByteArray& buf, int offset, int buf_size, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel, godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel) {
        return decode_delta(buf, offset, buf_size, out_id, out_pos, out_rot, out_vel, last_pos, last_rot, last_vel);
    }

    int ISyncable::parse_sync_packet_at(const godot::PackedByteArray& buf, int offset, uint32_t& out_id, godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel) {
        return decode_delta(buf, offset, (int)buf.size(), out_id, out_pos, out_rot, out_vel, delta_last_pos, delta_last_rot, delta_last_vel);
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
                float target = std::clamp(interp_step + JITTER_MARGIN * stddev, BUFFER_DELAY_MIN, BUFFER_DELAY_MAX);
                adaptive_delay = adaptive_delay * 0.95f + target * 0.05f;
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
                float extra = std::min(render_time - before -> time, interp_step * 3.0f);
                out_pos = before -> pos + before -> vel * extra;
            }
            else out_pos = before -> pos;
            out_rot = before -> rot;
            return;
        }

        float span = after -> time - before -> time;
        if (span <= 0.0f) { out_pos = after -> pos; out_rot = after -> rot; return; }
        float t = std::clamp((render_time - before -> time) / span, 0.0f, 1.0f);
        if (before -> pos.distance_to(after -> pos) > SNAP_THRESHOLD) {
            out_pos = after -> pos;
            out_rot = after -> rot;
        } 
        else {
            out_pos = before -> pos.lerp(after -> pos, t);
            out_rot = before -> rot.lerp(after -> rot, t);
        }
    }
}
