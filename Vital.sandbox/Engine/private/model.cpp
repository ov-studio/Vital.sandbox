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

// TODO: Improve
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
        reset_sync_state(); // ISyncable
        // sync registration happens on first poll() — safe from any thread
    }

    void Model::_notification(int what) {
        if (what == NOTIFICATION_PREDELETE) {
            // Always attempt unregister — poll() may have registered us
            // even if _ready() didn't (deferred registration pattern).
            Manager::Network::get_singleton()->unregister_syncable(this);
            sync_registered = false;
            if (on_destroyed_callback) on_destroyed_callback(this);
        }
    }


    //---------------------//
    //  ISyncable overrides //
    //---------------------//
    //
    // Delta encode/decode, snapshot buffer, jitter adaptation, and
    // interpolation all live in ISyncable (syncable.cpp). Model only
    // supplies the entity-specific hooks so PhysicsBody (and any future
    // syncable) can reuse the same path with zero copy-paste.

    void Model::apply_sync(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel) {
        if (!is_inside_tree()) return;

        // Authority peer drives its own transform — never overwrite.
        auto net = Manager::Network::get_singleton();
        if (net && net->get_peer_id() == sync_authority) return;

        sync_push_snapshot(pos, rot, vel);

        sync_last_pos = pos;
        sync_last_rot = rot;
        sync_last_vel = vel;
        sync_sleeping = false;
    }

    void Model::_process(double delta) {
        if (placeholder || !is_inside_tree() || !interp_ready) return;
        auto net = Manager::Network::get_singleton();
        if (net && net->get_peer_id() == sync_authority) return;
        // Delegate snapshot interpolation to ISyncable shared implementation.
        godot::Vector3 out_pos, out_rot;
        interp_process(delta, out_pos, out_rot);
        // When parented to another synced entity the snapshot values are in local
        // space, so write them back as local transform — not global.
        if (sync_parent_net_id != 0) {
            set_position(out_pos);
            set_rotation_degrees(out_rot);
        } else {
            set_global_position(out_pos);
            set_rotation_degrees(out_rot);
        }
    }

    void Model::on_sync_process(double delta) { _process(delta); }


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
            } 
            else {
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

    // Spawner is no longer used — replication is handled via _spawn_entity RPC.
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

            // Synchronous add_child — matches physics body behaviour so that
            // set_parent / set_global_position called in the same Lua tick see
            // the node already inside the scene tree.
            // _ready() and _notification() touch no Lua state, so there is no
            // re-entrancy risk from entering the tree here.
            object->add_child(instance);
            Core::get_singleton()->add_child(object);

            godot::UtilityFunctions::print("Model::create -> net_id=",
                object->net_id, " name=", Tool::to_godot_string(name));

            // Defer only the network side-effects (RPC + sync registration)
            // to the next frame so the _spawn_entity RPC goes out after _ready()
            // has run and the node is fully initialised on the server.
            uint32_t captured_net_id = object->net_id;
            godot::String captured_name = Tool::to_godot_string(name);
            int captured_authority = authority_peer;

            Core::get_singleton()->enqueue([object, captured_net_id,
                                            captured_name, captured_authority]() {
                Manager::Network::get_singleton()->enqueue_syncable_registration(object);

                auto net_node = Manager::Network::get_singleton()->get_node();
                if (net_node) {
                    net_node->rpc("_spawn_entity",
                        (int)captured_net_id,
                        (int)Engine::ISyncable::SyncType::Model,
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
            if (net_node) net_node->rpc("_destroy_entity", (int)net_id);
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
            // Use enqueue_syncable_registration — thread-safe pending queue.
            Manager::Network::get_singleton()->enqueue_syncable_registration(this);
        }
        set_visible(true);
        if (on_spawned_callback) on_spawned_callback(this, true);
        godot::UtilityFunctions::print("Model::hydrate — placeholder hydrated: ", Tool::to_godot_string(model_name));
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

    // ISyncable overrides
    bool Model::is_sync_active() const { return const_cast<Model*>(this)->is_inside_tree() && !placeholder; }

    godot::Vector3 Model::get_sync_position() const {
        if (!const_cast<Model*>(this)->is_inside_tree()) return godot::Vector3();
        // When parented to another synced entity, sync local position so the child
        // packet only carries the offset from the parent — not the full world coordinate.
        if (sync_parent_net_id != 0) return const_cast<Model*>(this)->get_position();
        return const_cast<Model*>(this)->get_global_position();
    }
    godot::Vector3 Model::get_sync_rotation() const {
        if (!const_cast<Model*>(this)->is_inside_tree()) return godot::Vector3();
        // Local rotation_degrees when parented — no change needed since Godot's
        // get_rotation_degrees() already returns local Euler angles.
        // When unparented it is also local (== global when parent is Core root),
        // so this is consistent in both cases.
        return const_cast<Model*>(this)->get_rotation_degrees();
    }
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



    // set_parent() / apply_parent() / get_parent_net_id() now live on
    // ISyncable (see syncable.cpp) — Model has no override here, it just
    // inherits the shared implementation.

    #if !defined(VSDK_Client)
    void Model::set_syncer(int peer_id) {
        sync_authority = (peer_id <= 1) ? 1 : peer_id;
        sync_sleeping  = false;
        godot::UtilityFunctions::print("Model net_id=", net_id,
            " set_syncer -> ", sync_authority);

        // Broadcast authority change reliably to all clients so each one
        // knows whether to run interpolation or raw local physics.
        // Broadcast authority change to all clients via generic _set_authority RPC.
        auto net_node = Manager::Network::get_singleton()->get_node();
        if (net_node) net_node->rpc("_set_authority", (int)net_id, sync_authority);
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
            } 
            else {
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
