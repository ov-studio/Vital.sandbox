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

    // Packet layout (28 bytes, all little-endian):
    //   [0..3]   uint32  net_id
    //   [4..7]   float   pos.x
    //   [8..11]  float   pos.y
    //   [12..15] float   pos.z
    //   [16..19] float   rot.x  (degrees)
    //   [20..23] float   rot.y
    //   [24..27] float   rot.z

    static constexpr int SYNC_PACKET_SIZE = 28;
    static constexpr float SYNC_RATE      = 1.0f / 20.0f;  // 20 Hz
    static constexpr float SYNC_THRESHOLD = 0.001f;         // metres / degrees

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

    // 28-byte server-auth broadcast packet (no sender prefix).
    static godot::PackedByteArray build_sync_packet(uint32_t id,
                                                     godot::Vector3 pos,
                                                     godot::Vector3 rot) {
        godot::PackedByteArray buf;
        buf.resize(SYNC_PACKET_SIZE);
        write_u32(buf,  0, id);
        write_float(buf,  4, pos.x);
        write_float(buf,  8, pos.y);
        write_float(buf, 12, pos.z);
        write_float(buf, 16, rot.x);
        write_float(buf, 20, rot.y);
        write_float(buf, 24, rot.z);
        return buf;
    }

    // 32-byte client-auth upload packet (sender_peer_id prefix + standard entry).
    // Server reads sender_peer_id from bytes [0..3] to validate authority without
    // needing MultiplayerAPI::get_remote_sender_id() (which is RPC-only).
    static godot::PackedByteArray build_client_sync_packet(uint32_t sender_peer_id,
                                                            uint32_t id,
                                                            godot::Vector3 pos,
                                                            godot::Vector3 rot) {
        godot::PackedByteArray buf;
        buf.resize(4 + SYNC_PACKET_SIZE); // 32 bytes
        write_u32(buf,  0, sender_peer_id);
        write_u32(buf,  4, id);
        write_float(buf,  8, pos.x);
        write_float(buf, 12, pos.y);
        write_float(buf, 16, pos.z);
        write_float(buf, 20, rot.x);
        write_float(buf, 24, rot.y);
        write_float(buf, 28, rot.z);
        return buf;
    }

    // static helpers — decode from raw bytes (used by Manager::Network)
    bool Model::parse_sync_packet_at(const godot::PackedByteArray& buf,
                                      int offset,
                                      uint32_t& out_id,
                                      godot::Vector3& out_pos,
                                      godot::Vector3& out_rot) {
        if (buf.size() < offset + SYNC_PACKET_SIZE) return false;
        out_id    = read_u32(buf,   offset);
        out_pos.x = read_float(buf, offset +  4);
        out_pos.y = read_float(buf, offset +  8);
        out_pos.z = read_float(buf, offset + 12);
        out_rot.x = read_float(buf, offset + 16);
        out_rot.y = read_float(buf, offset + 20);
        out_rot.z = read_float(buf, offset + 24);
        return true;
    }

    bool Model::parse_sync_packet(const godot::PackedByteArray& buf,
                                   uint32_t& out_id,
                                   godot::Vector3& out_pos,
                                   godot::Vector3& out_rot) {
        return parse_sync_packet_at(buf, 0, out_id, out_pos, out_rot);
    }

    // sync_tick removed — dirty-check, rate-limiting, and batching are
    // handled centrally in Manager::Network::poll() for all models at once.
    // This eliminates per-model RPC overhead and allows O(1) batched sends.

    void Model::apply_sync(godot::Vector3 pos, godot::Vector3 rot) {
        // Called from dispatch_sync_batch which runs inside the RPC handler on
        // the main thread — safe to apply directly, no enqueue needed.
        if (!is_inside_tree()) return;
        set_global_position(pos);
        set_rotation_degrees(rot);
        sync_last_pos = pos;
        sync_last_rot = rot;
        sync_sleeping = false;
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
        Engine::Core::get_singleton()->enqueue([this, position]() { set_global_position(position); });
    }

    void Model::set_rotation(godot::Vector3 rotation) {
        Engine::Core::get_singleton()->enqueue([this, rotation]() { set_rotation_degrees(rotation); });
    }

    #if !defined(VSDK_Client)
    void Model::set_syncer(int peer_id) {
        // peer_id == 0 or 1  -> server-authoritative
        // peer_id == N (>1)  -> client N is authoritative
        sync_authority = (peer_id <= 1) ? 1 : peer_id;
        sync_sleeping  = false; // wake so the authority change triggers a packet
        godot::UtilityFunctions::print("Model net_id=", net_id,
            " set_syncer -> ", sync_authority);
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
