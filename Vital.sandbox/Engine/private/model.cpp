/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: model.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Model Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/model.h>
#include <Vital.sandbox/Manager/public/asset.h>
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Manager/public/resource.h>


///////////////////////////
// Vital: Engine: Model //
///////////////////////////

// TOOD: Improve
namespace Vital::Engine {


    //------------------//
    //  Hooks / Notify  //
    //------------------//

    void Model::_ready() {
        if (placeholder) return;
        find_node(this, skeleton);
        find_node(this, anim_player);

        sync_authority = pending_authority;
        sync_last_pos  = get_global_position();
        sync_last_rot  = get_rotation_degrees();
        sync_sleeping  = false;
        sync_accum     = 0.0f;
        // Reset snapshot buffer and jitter state.
        snap_head  = 0;
        snap_count = 0;
        snap_clock = 0.0f;
        interp_ready      = false;
        jitter_last_arrival = -1.0f;
        jitter_idx        = 0;
        jitter_count      = 0;
        adaptive_delay    = BUFFER_DELAY;
        for (int i = 0; i < JITTER_WINDOW; i++) jitter_intervals[i] = 0.0f;
        // sync registration happens on first poll() — safe from any thread
    }

    void Model::_notification(int what) {
        if (what == NOTIFICATION_PREDELETE) {
            // Always attempt unregister — poll() may have registered us
            // even if _ready() didn't (deferred registration pattern).
            Manager::Network::get_singleton()->unregister_model(this);
            sync_registered = false;
            if (on_destroyed_callback) on_destroyed_callback(this);
        }
    }


    //---------------------//
    //  Low-level Sync     //
    //---------------------//

    // Delta-compressed variable-length packet layout:
    //   [0..3]   uint32   net_id
    //   [4..5]   uint16   component_mask  (bits 0-8 = px,py,pz,rx,ry,rz,vx,vy,vz)
    //   [6..]    float×N  only components where mask bit is set (4 bytes each)
    //
    // Minimum: 6 bytes (net_id + mask, nothing changed)
    // Maximum: 6 + 9*4 = 42 bytes (all components changed)
    // Typical moving object: ~18-26 bytes vs old fixed 40 bytes
    //
    // Receivers reconstruct the full state by applying received components
    // on top of their last-known values for this model.


    // Component mask bit positions
    static constexpr uint16_t MASK_PX = 1 << 0;
    static constexpr uint16_t MASK_PY = 1 << 1;
    static constexpr uint16_t MASK_PZ = 1 << 2;
    static constexpr uint16_t MASK_RX = 1 << 3;
    static constexpr uint16_t MASK_RY = 1 << 4;
    static constexpr uint16_t MASK_RZ = 1 << 5;
    static constexpr uint16_t MASK_VX = 1 << 6;
    static constexpr uint16_t MASK_VY = 1 << 7;
    static constexpr uint16_t MASK_VZ = 1 << 8;

    static void write_float(godot::PackedByteArray& buf, int offset, float v) {
        uint32_t raw;
        memcpy(&raw, &v, 4);
        buf[offset]   = raw & 0xFF;
        buf[offset+1] = (raw >> 8)  & 0xFF;
        buf[offset+2] = (raw >> 16) & 0xFF;
        buf[offset+3] = (raw >> 24) & 0xFF;
    }

    static float read_float(const godot::PackedByteArray& buf, int offset) {
        uint32_t raw = (uint8_t)buf[offset]
                     | ((uint8_t)buf[offset+1] << 8)
                     | ((uint8_t)buf[offset+2] << 16)
                     | ((uint8_t)buf[offset+3] << 24);
        float v;
        memcpy(&v, &raw, 4);
        return v;
    }

    static void write_u32(godot::PackedByteArray& buf, int offset, uint32_t v) {
        buf[offset]   = v & 0xFF;
        buf[offset+1] = (v >> 8)  & 0xFF;
        buf[offset+2] = (v >> 16) & 0xFF;
        buf[offset+3] = (v >> 24) & 0xFF;
    }

    static uint32_t read_u32(const godot::PackedByteArray& buf, int offset) {
        return (uint8_t)buf[offset]
             | ((uint8_t)buf[offset+1] << 8)
             | ((uint8_t)buf[offset+2] << 16)
             | ((uint8_t)buf[offset+3] << 24);
    }

    // Write u16 little-endian into buf at offset.
    static void write_u16(godot::PackedByteArray& buf, int offset, uint16_t v) {
        buf[offset]   =  v       & 0xFF;
        buf[offset+1] = (v >> 8) & 0xFF;
    }
    static uint16_t read_u16(const godot::PackedByteArray& buf, int offset) {
        return (uint8_t)buf[offset] | ((uint8_t)buf[offset+1] << 8);
    }

    // Encode a delta-compressed sync entry into buf starting at offset.
    // Only writes components that differ from last_* by more than threshold.
    // Updates last_* for components that are written.
    // Returns the number of bytes written.
    static int encode_delta(godot::PackedByteArray& buf, int offset,
                             uint32_t id,
                             godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel,
                             godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel) {
        uint16_t mask = 0;
        if (std::abs(pos.x - last_pos.x) > Model::DELTA_POS_THRESHOLD) mask |= MASK_PX;
        if (std::abs(pos.y - last_pos.y) > Model::DELTA_POS_THRESHOLD) mask |= MASK_PY;
        if (std::abs(pos.z - last_pos.z) > Model::DELTA_POS_THRESHOLD) mask |= MASK_PZ;
        if (std::abs(rot.x - last_rot.x) > Model::DELTA_ROT_THRESHOLD) mask |= MASK_RX;
        if (std::abs(rot.y - last_rot.y) > Model::DELTA_ROT_THRESHOLD) mask |= MASK_RY;
        if (std::abs(rot.z - last_rot.z) > Model::DELTA_ROT_THRESHOLD) mask |= MASK_RZ;
        if (std::abs(vel.x - last_vel.x) > Model::DELTA_VEL_THRESHOLD) mask |= MASK_VX;
        if (std::abs(vel.y - last_vel.y) > Model::DELTA_VEL_THRESHOLD) mask |= MASK_VY;
        if (std::abs(vel.z - last_vel.z) > Model::DELTA_VEL_THRESHOLD) mask |= MASK_VZ;

        write_u32(buf, offset, id);
        write_u16(buf, offset + 4, mask);
        int cursor = offset + 6;

        auto maybe_write = [&](bool bit, float val, float& last) {
            if (!bit) return;
            write_float(buf, cursor, val);
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

        return cursor - offset; // bytes written
    }

    // Decode a delta-compressed entry from buf at offset into out_*.
    // Caller must provide last_* (the receiver's last known values).
    // Returns bytes consumed, or -1 if buffer too small.
    static int decode_delta(const godot::PackedByteArray& buf, int offset, int buf_size,
                             uint32_t& out_id,
                             godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel,
                             godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel) {
        if (offset + 6 > buf_size) return -1;
        out_id         = read_u32(buf, offset);
        uint16_t mask  = read_u16(buf, offset + 4);
        int cursor     = offset + 6;

        // Start from last known values, overwrite only changed components.
        out_pos = last_pos;
        out_rot = last_rot;
        out_vel = last_vel;

        auto maybe_read = [&](bool bit, float& out, float& last) -> bool {
            if (!bit) return true;
            if (cursor + 4 > buf_size) return false;
            out   = read_float(buf, cursor);
            last  = out;
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

        return cursor - offset; // bytes consumed
    }

    // Decode a delta-compressed entry from buf at offset.
    // last_pos/rot/vel must be the receiver's last known values for this model
    // (kept in delta_last_* fields) so unchanged components are reconstructed.
    // Returns bytes consumed, or -1 on error.
    int Model::parse_sync_packet_at(const godot::PackedByteArray& buf,
                                     int offset,
                                     uint32_t& out_id,
                                     godot::Vector3& out_pos,
                                     godot::Vector3& out_rot,
                                     godot::Vector3& out_vel) {
        return decode_delta(buf, offset, (int)buf.size(),
                            out_id, out_pos, out_rot, out_vel,
                            delta_last_pos, delta_last_rot, delta_last_vel);
    }

    int Model::parse_sync_packet(const godot::PackedByteArray& buf,
                                  uint32_t& out_id,
                                  godot::Vector3& out_pos,
                                  godot::Vector3& out_rot,
                                  godot::Vector3& out_vel) {
        return parse_sync_packet_at(buf, 0, out_id, out_pos, out_rot, out_vel);
    }

    // Public static wrappers — called by Manager::Network (friend class).
    int Model::encode_delta_public(godot::PackedByteArray& buf, int offset,
                                    uint32_t id,
                                    godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel,
                                    godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel) {
        return encode_delta(buf, offset, id, pos, rot, vel, last_pos, last_rot, last_vel);
    }

    int Model::decode_delta_public(const godot::PackedByteArray& buf, int offset, int buf_size,
                                    uint32_t& out_id,
                                    godot::Vector3& out_pos, godot::Vector3& out_rot, godot::Vector3& out_vel,
                                    godot::Vector3& last_pos, godot::Vector3& last_rot, godot::Vector3& last_vel) {
        return decode_delta(buf, offset, buf_size, out_id, out_pos, out_rot, out_vel, last_pos, last_rot, last_vel);
    }

    // sync_tick removed — dirty-check, rate-limiting, and batching are
    // handled centrally in Manager::Network::poll() for all models at once.
    // This eliminates per-model RPC overhead and allows O(1) batched sends.

    void Model::apply_sync(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel) {
        if (!is_inside_tree()) return;

        // Authority peer drives its own transform — never overwrite.
        auto net = Manager::Network::get_singleton();
        if (net && net->get_peer_id() == sync_authority) return;

        if (!interp_ready) {
            set_global_position(pos);
            set_rotation_degrees(rot);
            snap_clock        = BUFFER_DELAY;
            jitter_last_arrival = snap_clock;
            interp_ready      = true;
        } else {
            // Measure inter-packet arrival interval for jitter estimation.
            float interval = snap_clock - jitter_last_arrival;
            if (interval > 0.0f) {
                jitter_intervals[jitter_idx] = interval;
                jitter_idx   = (jitter_idx + 1) % JITTER_WINDOW;
                if (jitter_count < JITTER_WINDOW) jitter_count++;

                // Compute mean and standard deviation over the sample window.
                float mean = 0.0f;
                for (int i = 0; i < jitter_count; i++) mean += jitter_intervals[i];
                mean /= (float)jitter_count;

                float variance = 0.0f;
                for (int i = 0; i < jitter_count; i++) {
                    float d = jitter_intervals[i] - mean;
                    variance += d * d;
                }
                float stddev = (jitter_count > 1)
                    ? std::sqrt(variance / (float)(jitter_count - 1))
                    : 0.0f;

                // Target delay = one packet interval + jitter_margin × stddev.
                // Smooth the transition so delay changes don't cause visible pops.
                float target = std::clamp(
                    interp_step + JITTER_MARGIN * stddev,
                    BUFFER_DELAY_MIN,
                    BUFFER_DELAY_MAX);
                // Exponential moving average — adapt slowly to avoid oscillation.
                adaptive_delay = adaptive_delay * 0.95f + target * 0.05f;
            }
            jitter_last_arrival = snap_clock;
        }

        // Write snapshot into ring buffer.
        // Timestamp in render-space: snap_clock - adaptive_delay.
        Snapshot& slot = snap_buf[snap_head];
        slot.pos       = pos;
        slot.rot       = rot;
        slot.vel       = vel;
        slot.time      = snap_clock - adaptive_delay;

        snap_head  = (snap_head + 1) % SNAPSHOT_COUNT;
        if (snap_count < SNAPSHOT_COUNT) snap_count++;

        sync_last_pos = pos;
        sync_last_rot = rot;
        sync_last_vel = vel;
        sync_sleeping = false;
    }

    void Model::_process(double delta) {
        if (placeholder || !is_inside_tree() || !interp_ready) return;

        // Authority peer drives its own transform directly.
        auto net = Manager::Network::get_singleton();
        if (net && net->get_peer_id() == sync_authority) return;

        // Advance local clock.
        snap_clock += static_cast<float>(delta);

        if (snap_count == 0) return;

        // Render point in render-space time (snap_clock - BUFFER_DELAY).
        // Snapshot timestamps are stored as (snap_clock - BUFFER_DELAY) at write
        // time, so render_time directly indexes into the buffer without offset.
        float render_time = snap_clock - adaptive_delay;

        // Read the ring buffer in chronological order.
        // snap_buf is a circular buffer; oldest entry is at:
        //   (snap_head - snap_count + SNAPSHOT_COUNT) % SNAPSHOT_COUNT
        // Find the two snapshots that bracket render_time.
        const Snapshot* before = nullptr;
        const Snapshot* after  = nullptr;

        for (int i = 0; i < snap_count; i++) {
            int idx = (snap_head - snap_count + i + SNAPSHOT_COUNT) % SNAPSHOT_COUNT;
            const Snapshot& s = snap_buf[idx];
            if (s.time <= render_time) before = &s;
            else if (!after)           after  = &s;
        }

        if (!before && !after) return;

        if (!before) {
            // render_time is before all snapshots — snap to oldest.
            set_global_position(after->pos);
            set_rotation_degrees(after->rot);
            return;
        }

        if (!after) {
            // render_time is past all snapshots — extrapolate from newest.
            // Only extrapolate if model was moving; otherwise hold position.
            // Cap at 3 × interp_step to bound snap-back when next packet arrives.
            if (before->vel.length() > VEL_THRESHOLD) {
                float extra = std::min(render_time - before->time, interp_step * 3.0f);
                set_global_position(before->pos + before->vel * extra);
            } else {
                set_global_position(before->pos);
            }
            set_rotation_degrees(before->rot);
            return;
        }

        // Normal case — render_time sits between `before` and `after`.
        float span = after->time - before->time;
        if (span <= 0.0f) {
            set_global_position(after->pos);
            set_rotation_degrees(after->rot);
            return;
        }

        float t = (render_time - before->time) / span;
        t = std::clamp(t, 0.0f, 1.0f);

        // Teleport detection: if the gap between snapshots is huge the model
        // was teleported — snap rather than lerp across an impossible distance.
        float dist = before->pos.distance_to(after->pos);
        if (dist > SNAP_THRESHOLD) {
            set_global_position(after->pos);
            set_rotation_degrees(after->rot);
        } else {
            set_global_position(before->pos.lerp(after->pos, t));
            set_rotation_degrees(before->rot.lerp(after->rot, t));
        }
    }


    //---------------------------//
    //  Mesh / Material Helpers  //
    //---------------------------//

    godot::MeshInstance3D* Model::find_mesh_node(godot::Node* node, const std::string& path) {
        if (!node) return nullptr;
        auto separator = path.find('/');
        std::string segment   = (separator == std::string::npos) ? path : path.substr(0, separator);
        std::string remainder = (separator == std::string::npos) ? ""   : path.substr(separator + 1);
        for (int i = 0; i < node->get_child_count(); i++) {
            godot::Node* child = node->get_child(i);
            std::string child_name = Tool::to_std_string(child->get_name());
            if (!child_name.empty() && child_name[0] == '@') {
                auto result = find_mesh_node(child, path);
                if (result) return result;
                continue;
            }
            if (child_name != segment) continue;
            if (remainder.empty()) {
                auto mesh = godot::Object::cast_to<godot::MeshInstance3D>(child);
                if (mesh) return mesh;
            } else {
                auto result = find_mesh_node(child, remainder);
                if (result) return result;
            }
        }
        return nullptr;
    }

    int Model::find_material_index(godot::MeshInstance3D* mesh, const std::string& material) {
        godot::ArrayMesh* array_mesh = godot::Object::cast_to<godot::ArrayMesh>(mesh->get_mesh().ptr());
        if (!array_mesh) return -1;
        for (int i = 0; i < array_mesh->get_surface_count(); i++) {
            if (Tool::to_std_string(array_mesh->surface_get_name(i)) == material) return i;
        }
        return -1;
    }

    void Model::collect_mesh_nodes(godot::Node* node, std::vector<std::string>& out, const std::string& current_path) {
        if (!node) return;
        for (int i = 0; i < node->get_child_count(); i++) {
            godot::Node* child = node->get_child(i);
            std::string child_name = Tool::to_std_string(child->get_name());
            bool is_generated = !child_name.empty() && child_name[0] == '@';
            std::string child_path = (current_path.empty() || is_generated) ? (is_generated ? "" : child_name) : current_path + "/" + child_name;
            if (!is_generated && godot::Object::cast_to<godot::MeshInstance3D>(child)) out.push_back(child_path);
            collect_mesh_nodes(child, out, child_path);
        }
    }


    //----------//
    // Asserts  //
    //----------//

    godot::MeshInstance3D* Model::assert_component(const std::string& component) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
            fmt::format("component '{}' not found in model '{}'", component, model_name));
        return mesh;
    }

    std::pair<godot::MeshInstance3D*, int> Model::assert_material(const std::string& component, const std::string& material) {
        auto mesh = assert_component(component);
        int idx = find_material_index(mesh, material);
        if (idx < 0) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
            fmt::format("material '{}' not found in component '{}'", material, component));
        return { mesh, idx };
    }

    godot::Skeleton3D* Model::assert_skeleton() {
        if (!skeleton) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
            fmt::format("no skeleton found in model '{}'", model_name));
        return skeleton;
    }

    godot::AnimationPlayer* Model::assert_animation_player() {
        if (!anim_player) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
            fmt::format("no animation player found in model '{}'", model_name));
        return anim_player;
    }

    void Model::assert_material_feature(int feature) {
        if (feature < 0 || feature >= godot::BaseMaterial3D::FEATURE_MAX)
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "invalid material feature");
    }

    void Model::assert_material_flag(int flag) {
        if (flag < 0 || flag >= godot::BaseMaterial3D::FLAG_MAX)
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "invalid material flag");
    }


    //---------------------------//
    //  Spawner Setup (private)  //
    //---------------------------//

    // Spawner is no longer used — replication is handled via _spawn_model RPC.
    // These stubs are kept so existing call-sites (resource manager etc.) compile.
    void Model::setup_spawner()   {}
    void Model::teardown_spawner() {}

    void Model::cleanup_spawned() {
        auto core = Engine::Core::get_singleton();
        if (!core) return;
        for (int i = core->get_child_count() - 1; i >= 0; i--) {
            godot::Node* child = core->get_child(i);
            if (godot::Object::cast_to<Model>(child)) child->queue_free();
        }
    }

    void Model::on_connected() {
        Manager::Asset::get_singleton()->clear();
        cleanup_spawned();
        next_net_id = 1;
    }


    //------------------//
    //    Managers      //
    //------------------//

    bool Model::load(const std::string& name, const std::string& path) {
        return load_from_buffer(name, Tool::File::read_binary(Tool::get_directory(), path));
    }

    bool Model::load_from_buffer(const std::string& name, const godot::PackedByteArray& buffer) {
        if (is_model_loaded(name)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
            fmt::format("model '{}' is already loaded", name));

        godot::Ref<godot::PackedScene> scene;
        switch (Tool::Format::get_format(format_registry, Format::UNKNOWN, buffer)) {
            case Format::GLB: {
                godot::Ref<godot::GLTFDocument> document = memnew(godot::GLTFDocument);
                godot::Ref<godot::GLTFState>    state    = memnew(godot::GLTFState);
                if (document->append_from_buffer(buffer, "", state) != godot::OK)
                    throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "invalid model buffer");
                godot::Node* root = document->generate_scene(state);
                if (!root) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "failed to generate scene");
                scene = godot::Ref<godot::PackedScene>(memnew(godot::PackedScene));
                scene->pack(root);
                memdelete(root);
                break;
            }
            default: break;
        }

        if (scene.is_null()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
            "unsupported or invalid model format");
        cache_loaded[name] = scene;
        #if defined(VSDK_Client)
        Manager::Asset::get_singleton()->flush_spawn_queue(name);
        #endif
        return true;
    }

    bool Model::unload(const std::string& name) {
        auto it = cache_loaded.find(name);
        if (it == cache_loaded.end()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
            fmt::format("model '{}' isn't loaded yet", name));
        cache_loaded.erase(it);
        return true;
    }

    Model* Model::create(const std::string& name, int authority_peer) {
        auto it = cache_loaded.find(name);
        if (it == cache_loaded.end()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
            fmt::format("model '{}' isn't loaded yet", name));

        #if defined(VSDK_Client)
            Model* object = memnew(Model);
            object->set_model_name(name);
            godot::Node* instance = it->second->instantiate();
            if (!instance) {
                memdelete(object);
                throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
                    fmt::format("failed to instantiate model '{}'", name));
            }
            object->add_child(instance);
            Engine::Core::get_singleton()->add_child(object);
            return object;
        #else
            // Server-side: create the model locally, assign net_id before
            // add_child so _ready() sees it, then RPC clients to mirror it.
            Model* object = memnew(Model);
            object->net_id            = next_net_id++;
            object->pending_authority = authority_peer;

            godot::Node* instance = it->second->instantiate();
            if (!instance) {
                memdelete(object);
                throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
                    fmt::format("failed to instantiate model '{}'", name));
            }
            object->set_model_name(name);

            // Defer add_child to next frame — adding to the scene tree during a
            // Lua C call can trigger deferred signals that re-enter Lua and corrupt
            // get_environment_id(). Poll() will pick up the registration lazily.
            uint32_t captured_net_id = object->net_id;
            godot::String captured_name = Tool::to_godot_string(name);
            int captured_authority = authority_peer;

            Core::get_singleton()->enqueue([object, instance, captured_net_id,
                                            captured_name, captured_authority]() {
                object->add_child(instance);
                Core::get_singleton()->add_child(object);

                godot::UtilityFunctions::print("Model::create (deferred) -> net_id=",
                    captured_net_id, " name=", captured_name);

                // Use enqueue_model_registration so poll() picks it up via the
                // pending queue rather than the O(N) child scan.
                Manager::Network::get_singleton()->enqueue_model_registration(object);

                auto net_node = Manager::Network::get_singleton()->get_node();
                if (net_node) {
                    net_node->rpc("_spawn_model",
                        (int)captured_net_id,
                        captured_name,
                        captured_authority);
                }

                if (Model::on_spawned_callback) Model::on_spawned_callback(object, false);
            });

            return object;
        #endif
    }

    void Model::destroy() {
        #if !defined(VSDK_Client)
        // Notify all clients to remove their copy before we free server-side.
        if (net_id != 0) {
            auto net_node = Manager::Network::get_singleton()->get_node();
            if (net_node) net_node->rpc("_destroy_model", (int)net_id);
        }
        #endif
        this->queue_free();
    }

    #if defined(VSDK_Client)
    void Model::hydrate(int authority_peer) {
        if (!placeholder) return;
        auto it = cache_loaded.find(model_name);
        if (it == cache_loaded.end()) {
            godot::UtilityFunctions::push_warning("Model::hydrate — model not in cache: ",
                Tool::to_godot_string(model_name));
            return;
        }
        godot::Node* instance = it->second->instantiate();
        if (!instance) {
            godot::UtilityFunctions::push_warning("Model::hydrate — failed to instantiate: ",
                Tool::to_godot_string(model_name));
            return;
        }
        placeholder      = false;
        pending_authority = authority_peer;
        remote           = true;
        add_child(instance);
        find_node(this, skeleton);
        find_node(this, anim_player);
        sync_authority = authority_peer;
        sync_last_pos  = get_global_position();
        sync_last_rot  = get_rotation_degrees();
        if (!sync_registered) {
            // hydrate() runs on the main thread (asset download callback),
            // use enqueue_model_registration for consistency with the pending queue.
            Manager::Network::get_singleton()->enqueue_model_registration(this);
        }
        set_visible(true);
        if (on_spawned_callback) on_spawned_callback(this, true);
        godot::UtilityFunctions::print("Model::hydrate — placeholder hydrated: ",
            Tool::to_godot_string(model_name));
    }
    #endif

    std::vector<std::string> Model::filter_resource_models(const std::string& resource, const std::vector<std::string>& files) {
        std::vector<std::string> validated;
        for (const auto& file : files) {
            if (!Tool::Format::is_supported_extension(format_registry, file)) continue;
            if (!Tool::Format::is_supported_format(format_registry, Format::UNKNOWN,
                    fmt::format("resources/{}/{}", resource, file))) continue;
            validated.push_back(file);
        }
        return validated;
    }

    void Model::load_resource_models(const std::string& resource, const std::vector<std::string>& files) {
        auto rm = Vital::Manager::Resource::get_singleton();
        std::vector<std::string> loaded, failed;
        for (const auto& file : files) {
            if (!Tool::Format::is_supported_extension(format_registry, file)) continue;
            const std::string mn       = fmt::format(":{}/{}", resource, file);
            const std::string lp       = fmt::format("resources/{}/{}", resource, file);
            if (is_model_loaded(mn)) continue;
            if (!Tool::Format::is_supported_format(format_registry, Format::UNKNOWN, lp)) continue;
            try { load(mn, lp); loaded.push_back(mn); }
            catch (...) { failed.push_back(file); }
        }
        if (!loaded.empty()) {
            std::string report = fmt::format("resource `{}` registered {} model asset(s):\n", resource, loaded.size());
            for (const auto& n : loaded) report += fmt::format("> `{}`\n", n);
            rm->log("sbox", report);
        }
        if (!failed.empty()) {
            std::string report = fmt::format("resource `{}` failed to load {} model asset(s):\n", resource, failed.size());
            for (const auto& n : failed) report += fmt::format("> `{}`\n", n);
            rm->log("error", report);
        }
    }

    void Model::unload_resource_models(const std::string& resource) {
        auto rm = Vital::Manager::Resource::get_singleton();
        const std::string prefix = fmt::format(":{}/", resource);
        std::vector<std::string> to_unload;
        for (const auto& [name, _] : cache_loaded) {
            if (name.rfind(prefix, 0) == 0) to_unload.push_back(name);
        }
        for (const auto& name : to_unload) { try { unload(name); } catch (...) {} }
        if (!to_unload.empty()) rm->log("sbox",
            fmt::format("resource `{}` unloaded {} model asset(s)", resource, to_unload.size()));
    }


    //-------------//
    //  Checkers   //
    //-------------//

    bool Model::is_model_loaded(const std::string& name) { return cache_loaded.find(name) != cache_loaded.end(); }
    bool Model::is_remote() const { return remote; }

    bool Model::is_streamed() const {
        if (placeholder || !is_visible_in_tree()) return false;
        #if defined(VSDK_Client)
            auto camera = Engine::Core::get_scene_root()->get_camera_3d();
            if (!camera) return false;
            return camera->is_position_in_frustum(get_global_position());
        #else
            return true;
        #endif
    }

    bool Model::is_component_visible(const std::string& component) { return assert_component(component)->is_visible(); }

    bool Model::is_material_visible(const std::string& component, const std::string& material) {
        auto [mesh, idx] = assert_material(component, material);
        return !mesh->get_surface_override_material(idx).is_valid();
    }

    bool Model::is_material_feature(const std::string& component, const std::string& material, int feature) {
        assert_material_feature(feature);
        auto [mesh, idx] = assert_material(component, material);
        godot::Ref<godot::StandardMaterial3D> std_mat =
            godot::Object::cast_to<godot::StandardMaterial3D>(mesh->get_active_material(idx).ptr());
        if (!std_mat.is_valid()) return false;
        return std_mat->get_feature(static_cast<godot::BaseMaterial3D::Feature>(feature));
    }

    bool Model::is_material_flag(const std::string& component, const std::string& material, int flag) {
        assert_material_flag(flag);
        auto [mesh, idx] = assert_material(component, material);
        godot::Ref<godot::StandardMaterial3D> std_mat =
            godot::Object::cast_to<godot::StandardMaterial3D>(mesh->get_active_material(idx).ptr());
        if (!std_mat.is_valid()) return false;
        return std_mat->get_flag(static_cast<godot::BaseMaterial3D::Flags>(flag));
    }

    bool Model::is_animation_playing() { return anim_player && anim_player->is_playing(); }


    //------------//
    //  Getters   //
    //------------//

    Model::Models Model::get_loaded_models() { return cache_loaded; }
    std::string   Model::get_model_name()     { return model_name; }
    godot::Vector3 Model::get_position()      { return is_inside_tree() ? get_global_position() : godot::Vector3(); }
    godot::Vector3 Model::get_rotation()      { return get_rotation_degrees(); }
    int Model::get_sync_authority() const     { return sync_authority; }
    uint32_t Model::get_net_id() const        { return net_id; }

    std::vector<std::string> Model::get_components() {
        std::vector<std::string> components;
        collect_mesh_nodes(this, components, "");
        return components;
    }

    std::vector<std::string> Model::get_materials(const std::string& component) {
        godot::ArrayMesh* array_mesh =
            godot::Object::cast_to<godot::ArrayMesh>(assert_component(component)->get_mesh().ptr());
        std::vector<std::string> materials;
        if (!array_mesh) return materials;
        for (int i = 0; i < array_mesh->get_surface_count(); i++)
            materials.push_back(Tool::to_std_string(array_mesh->surface_get_name(i)));
        return materials;
    }

    std::vector<std::string> Model::get_blendshapes(const std::string& component) {
        auto mesh = assert_component(component);
        godot::ArrayMesh* array_mesh = godot::Object::cast_to<godot::ArrayMesh>(mesh->get_mesh().ptr());
        std::vector<std::string> blendshapes;
        if (!array_mesh) return blendshapes;
        for (int i = 0; i < mesh->get_blend_shape_count(); i++)
            blendshapes.push_back(Tool::to_std_string(array_mesh->get_blend_shape_name(i)));
        return blendshapes;
    }

    std::vector<std::string> Model::get_bones() {
        std::vector<std::string> bones;
        if (skeleton) for (int i = 0; i < skeleton->get_bone_count(); i++)
            bones.push_back(Tool::to_std_string(skeleton->get_bone_name(i)));
        return bones;
    }

    std::vector<std::string> Model::get_animations() {
        std::vector<std::string> animations;
        if (anim_player) {
            auto list = anim_player->get_animation_list();
            for (int i = 0; i < list.size(); i++)
                animations.push_back(Tool::to_std_string(list[i]));
        }
        return animations;
    }

    float Model::get_blendshape_value(const std::string& component, const std::string& blend_shape) {
        auto mesh = assert_component(component);
        int idx = mesh->find_blend_shape_by_name(Tool::to_godot_string(blend_shape));
        if (idx < 0) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
            fmt::format("blendshape '{}' not found in component '{}'", blend_shape, component));
        return mesh->get_blend_shape_value(idx);
    }

    godot::Vector3 Model::get_bone_position(const std::string& bone) {
        auto skel = assert_skeleton();
        int idx = skel->find_bone(Tool::to_godot_string(bone));
        if (idx == -1) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
            fmt::format("bone '{}' not found in model '{}'", bone, model_name));
        return skel->get_global_transform().xform(skel->get_bone_global_pose(idx).origin);
    }

    std::string Model::get_current_animation() {
        return Tool::to_std_string(assert_animation_player()->get_current_animation());
    }

    float Model::get_animation_speed() { return assert_animation_player()->get_speed_scale(); }


    //------------//
    //  Setters   //
    //------------//

    void Model::set_model_name(const std::string& name) { model_name = name; }

    void Model::set_position(godot::Vector3 position) {
        #if defined(VSDK_Client)
        Engine::Core::get_singleton()->enqueue([this, position]() { set_global_position(position); });
        #else
        // Server is always on the main thread — apply directly so get_position()
        // returns the correct value immediately in the same Lua timer tick.
        if (is_inside_tree()) set_global_position(position);
        else sync_last_pos = position;
        #endif
    }

    void Model::set_rotation(godot::Vector3 rotation) {
        #if defined(VSDK_Client)
        Engine::Core::get_singleton()->enqueue([this, rotation]() { set_rotation_degrees(rotation); });
        #else
        if (is_inside_tree()) set_rotation_degrees(rotation);
        else sync_last_rot = rotation;
        #endif
    }

    #if !defined(VSDK_Client)
    void Model::set_syncer(int peer_id) {
        sync_authority = (peer_id <= 1) ? 1 : peer_id;
        sync_sleeping  = false;
        godot::UtilityFunctions::print("Model net_id=", net_id,
            " set_syncer -> ", sync_authority);

        // Broadcast authority change reliably to all clients so each one
        // knows whether to run interpolation or raw local physics.
        auto net_node = Manager::Network::get_singleton()->get_node();
        if (net_node) {
            net_node->rpc("_set_authority", (int)net_id, sync_authority);
        }
    }
    #endif

    bool Model::set_component_visible(const std::string& component, bool state) {
        auto exec = [&](const std::string& name) -> bool {
            godot::MeshInstance3D* mesh = find_mesh_node(this, name);
            if (!mesh) return false;
            mesh->set_visible(state);
            return true;
        };
        if (!apply_wildcard(component, [&]{ return get_components(); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
                fmt::format("component '{}' not found in model '{}'", component, model_name));
        return true;
    }

    bool Model::set_material_visible(const std::string& component, const std::string& material, bool state) {
        auto mesh = assert_component(component);
        auto exec = [&](const std::string& name) -> bool {
            int idx = find_material_index(mesh, name);
            if (idx < 0) return false;
            if (!state) {
                godot::Ref<godot::StandardMaterial3D> invisible(memnew(godot::StandardMaterial3D));
                invisible->set_transparency(godot::BaseMaterial3D::TRANSPARENCY_ALPHA);
                invisible->set_depth_draw_mode(godot::BaseMaterial3D::DEPTH_DRAW_DISABLED);
                invisible->set_albedo(godot::Color(0, 0, 0, 0));
                mesh->set_surface_override_material(idx, invisible);
            } else {
                mesh->set_surface_override_material(idx, godot::Ref<godot::Material>());
            }
            return true;
        };
        if (!apply_wildcard(material, [&]{ return get_materials(component); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
                fmt::format("material '{}' not found in component '{}'", material, component));
        return true;
    }

    bool Model::set_material_feature(const std::string& component, const std::string& material, int feature, bool state) {
        assert_material_feature(feature);
        auto mesh = assert_component(component);
        auto exec = [&](const std::string& name) -> bool {
            return apply_standard_material(mesh, find_material_index(mesh, name),
                [&](godot::Ref<godot::StandardMaterial3D> mat) {
                    mat->set_feature(static_cast<godot::BaseMaterial3D::Feature>(feature), state);
                });
        };
        if (!apply_wildcard(material, [&]{ return get_materials(component); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
                fmt::format("material '{}' not found in component '{}'", material, component));
        return true;
    }

    bool Model::set_material_flag(const std::string& component, const std::string& material, int flag, bool state) {
        assert_material_flag(flag);
        auto mesh = assert_component(component);
        auto exec = [&](const std::string& name) -> bool {
            return apply_standard_material(mesh, find_material_index(mesh, name),
                [&](godot::Ref<godot::StandardMaterial3D> mat) {
                    mat->set_flag(static_cast<godot::BaseMaterial3D::Flags>(flag), state);
                });
        };
        if (!apply_wildcard(material, [&]{ return get_materials(component); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
                fmt::format("material '{}' not found in component '{}'", material, component));
        return true;
    }

    bool Model::set_blendshape_value(const std::string& component, const std::string& blend_shape, float value) {
        auto mesh = assert_component(component);
        auto exec = [&](const std::string& name) -> bool {
            int idx = mesh->find_blend_shape_by_name(Tool::to_godot_string(name));
            if (idx < 0) return false;
            mesh->set_blend_shape_value(idx, value);
            return true;
        };
        if (!apply_wildcard(blend_shape, [&]{ return get_blendshapes(component); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
                fmt::format("blendshape '{}' not found in component '{}'", blend_shape, component));
        return true;
    }

    void Model::set_animation_speed(float speed) { assert_animation_player()->set_speed_scale(speed); }


    //---------//
    //  Misc   //
    //---------//

    bool Model::play_animation(const std::string& name, bool loop, float speed) {
        if (!anim_player) return false;
        if (!anim_player->has_animation(Tool::to_godot_string(name))) {
            godot::UtilityFunctions::push_warning("Animation '", Tool::to_godot_string(name),
                "' not found in model '", Tool::to_godot_string(model_name), "'");
            return false;
        }
        godot::Ref<godot::Animation> animation = anim_player->get_animation(Tool::to_godot_string(name));
        if (animation.is_valid())
            animation->set_loop_mode(loop ? godot::Animation::LOOP_LINEAR : godot::Animation::LOOP_NONE);
        anim_player->set_speed_scale(speed);
        anim_player->play(Tool::to_godot_string(name));
        return true;
    }

    void Model::stop_animation()   { if (anim_player) anim_player->stop(); }
    void Model::pause_animation()  { if (anim_player) anim_player->pause(); }
    void Model::resume_animation() {
        if (!anim_player) return;
        auto current = anim_player->get_current_animation();
        if (current.is_empty()) return;
        anim_player->play(current);
    }
}
