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
        // Reset snapshot buffer.
        snap_head  = 0;
        snap_count = 0;
        snap_clock = 0.0f;
        interp_ready = false;
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

    // Packet layout (40 bytes, all little-endian):
    //   [0..3]   uint32  net_id
    //   [4..15]  float×3 pos xyz
    //   [16..27] float×3 rot xyz (degrees)
    //   [28..39] float×3 vel xyz (units/sec — for dead-reckoning on receiver)

    static constexpr int SYNC_PACKET_SIZE = 40;
    static constexpr float SYNC_THRESHOLD = 0.001f;

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

    // 40-byte server-auth broadcast packet: net_id + pos + rot + vel.
    static godot::PackedByteArray build_sync_packet(uint32_t id,
                                                     godot::Vector3 pos,
                                                     godot::Vector3 rot,
                                                     godot::Vector3 vel) {
        godot::PackedByteArray buf;
        buf.resize(SYNC_PACKET_SIZE);
        write_u32(buf,  0, id);
        write_float(buf,  4, pos.x); write_float(buf,  8, pos.y); write_float(buf, 12, pos.z);
        write_float(buf, 16, rot.x); write_float(buf, 20, rot.y); write_float(buf, 24, rot.z);
        write_float(buf, 28, vel.x); write_float(buf, 32, vel.y); write_float(buf, 36, vel.z);
        return buf;
    }

    // 44-byte client-auth upload: sender_peer_id prefix + 40-byte entry.
    static godot::PackedByteArray build_client_sync_packet(uint32_t sender_peer_id,
                                                            uint32_t id,
                                                            godot::Vector3 pos,
                                                            godot::Vector3 rot,
                                                            godot::Vector3 vel) {
        godot::PackedByteArray buf;
        buf.resize(4 + SYNC_PACKET_SIZE); // 44 bytes
        write_u32(buf,  0, sender_peer_id);
        write_u32(buf,  4, id);
        write_float(buf,  8, pos.x); write_float(buf, 12, pos.y); write_float(buf, 16, pos.z);
        write_float(buf, 20, rot.x); write_float(buf, 24, rot.y); write_float(buf, 28, rot.z);
        write_float(buf, 32, vel.x); write_float(buf, 36, vel.y); write_float(buf, 40, vel.z);
        return buf;
    }

    // static helpers — decode 40-byte entries (net_id + pos + rot + vel)
    bool Model::parse_sync_packet_at(const godot::PackedByteArray& buf,
                                      int offset,
                                      uint32_t& out_id,
                                      godot::Vector3& out_pos,
                                      godot::Vector3& out_rot,
                                      godot::Vector3& out_vel) {
        if (buf.size() < offset + SYNC_PACKET_SIZE) return false;
        out_id    = read_u32(buf,   offset);
        out_pos.x = read_float(buf, offset +  4);
        out_pos.y = read_float(buf, offset +  8);
        out_pos.z = read_float(buf, offset + 12);
        out_rot.x = read_float(buf, offset + 16);
        out_rot.y = read_float(buf, offset + 20);
        out_rot.z = read_float(buf, offset + 24);
        out_vel.x = read_float(buf, offset + 28);
        out_vel.y = read_float(buf, offset + 32);
        out_vel.z = read_float(buf, offset + 36);
        return true;
    }

    bool Model::parse_sync_packet(const godot::PackedByteArray& buf,
                                   uint32_t& out_id,
                                   godot::Vector3& out_pos,
                                   godot::Vector3& out_rot,
                                   godot::Vector3& out_vel) {
        return parse_sync_packet_at(buf, 0, out_id, out_pos, out_rot, out_vel);
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
            // First snapshot — snap immediately and pre-seed the clock so the
            // render point (snap_clock - BUFFER_DELAY) is immediately valid.
            // Without this, the buffer spends the first 100ms in underrun and
            // the model visually slips as it transitions from snap to lerp mode.
            set_global_position(pos);
            set_rotation_degrees(rot);
            snap_clock   = BUFFER_DELAY; // render_time = 0.0 on first frame
            interp_ready = true;
        }

        // Write snapshot into the ring buffer. Timestamp after clock seed so
        // the first slot.time == 0.0 which matches render_time on the first frame.
        Snapshot& slot = snap_buf[snap_head];
        slot.pos       = pos;
        slot.rot       = rot;
        slot.vel       = vel;
        slot.time      = snap_clock - BUFFER_DELAY; // convert to render-space time

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
        float render_time = snap_clock - BUFFER_DELAY;

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
