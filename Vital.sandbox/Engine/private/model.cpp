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

    void Model::apply_sync(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3 scale) {
        if (!is_inside_tree()) return;

        // Authority peer drives its own transform — never overwrite.
        auto net = Manager::Network::get_singleton();
        if (net && net->get_peer_id() == sync_authority) return;

        sync_push_snapshot(pos, rot, vel);
        // Scale snaps (not interpolated) — rare changes, exact value matters.
        set_scale(scale);

        sync_last_pos = pos;
        sync_last_rot = rot;
        sync_last_vel = vel;
        sync_last_scale = scale;
        sync_sleeping = false;
    }

    godot::Vector3 Model::get_sync_scale() const {
        if (!const_cast<Model*>(this)->is_inside_tree()) return godot::Vector3(1, 1, 1);
        return const_cast<Model*>(this)->get_scale();
    }

    void Model::_process(double delta) {
        // Layer weight tweening runs regardless of sync authority/placeholder
        // state — it's purely a local visual concern (every peer that has
        // this model rendered needs its overlay layers to blend smoothly).
        if (anim_tree) update_animation_layers((float)delta);

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
        const std::string file_hash = Tool::File::hash(Tool::get_directory(), path);
        // Same path, new bytes: drop old PackedScene so generate_scene runs again.
        if (is_model_loaded(name)) {
            auto hit = cache_hashes.find(name);
            if (hit != cache_hashes.end() && hit->second == file_hash)
                return true;
            unload(name);
        }
        if (!load_from_buffer(name, Tool::File::read_binary(Tool::get_directory(), path)))
            return false;
        cache_hashes[name] = file_hash;
        return true;
    }

    bool Model::load_from_buffer(const std::string& name, const godot::PackedByteArray& buffer) {
        // Serialize imports: concurrent/rapid restart must not interleave
        // GLTFDocument work (GPU/driver heap corruption in generate_scene).
        static std::mutex load_mutex;
        std::lock_guard<std::mutex> load_lock(load_mutex);

        if (is_model_loaded(name)) return true;

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
                if (scene->pack(root) != godot::OK) {
                    memdelete(root);
                    throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "failed to pack model scene");
                }
                memdelete(root);
                // Drop GLTF objects before releasing lock so the next import
                // does not overlap document/state teardown.
                document.unref();
                state.unref();
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
        if (it == cache_loaded.end()) return false;
        // Drop cache entry only. Live instances keep their own node trees;
        // pending hydrates must be cancelled via Asset::spawn_generation first.
        cache_loaded.erase(it);
        cache_hashes.erase(name);
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
            // Same UAF risk as Physics_Body::setup_create() had: this lambda
            // runs on a later drain() of Core's work queue, so a raw `object`
            // capture would be dangling if Model::destroy() runs first.
            // Capture by ObjectID and re-validate through ObjectDB instead.
            godot::ObjectID captured_oid = godot::ObjectID(object->get_instance_id());

            Core::get_singleton()->enqueue([captured_oid, captured_net_id,
                                            captured_name]() {
                godot::Object* obj = godot::ObjectDB::get_instance(captured_oid);
                if (!obj) return; // destroyed before this deferred registration ran
                auto* object = godot::Object::cast_to<Model>(obj);
                if (!object) return;

                Manager::Network::get_singleton()->enqueue_syncable_registration(object);

                auto net_node = Manager::Network::get_singleton()->get_node();
                if (net_node) {
                    // Read sync_authority fresh here rather than using the
                    // authority_peer captured at create() time — if set_syncer()
                    // was called in the same Lua tick as create() (before this
                    // deferred registration ran), it already wrote the new
                    // authority directly onto this object (see Model::set_syncer,
                    // which skips its own broadcast in that case precisely so
                    // this single _spawn_entity RPC is the one source of truth).
                    // Always send WORLD transform for spawn. get_sync_position()
                    // is local when parented; clients apply init as global before
                    // _reparent_entity, which would plant the mesh at the local
                    // offset in world space (ball meshes stuck at origin while
                    // colliders sit on the field).
                    godot::Vector3 spawn_pos = object->get_global_position();
                    godot::Vector3 spawn_rot = object->get_global_rotation_degrees();
                    net_node->rpc("_spawn_entity",
                        (int)captured_net_id,
                        (int)Engine::ISyncable::SyncType::Model,
                        captured_name,
                        object->get_sync_authority(),
                        spawn_pos,
                        spawn_rot);
                }

                // Flush any set_position/set_rotation that was called in the
                // same Lua tick as create() — net_id was 0 then so
                // force_transform_broadcast() stashed it instead of sending.
                // Now that _spawn_entity is out, fire _force_transform to the
                // owning peer so it applies the override and reseeds its baseline.
                object->flush_pending_force_transform();

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
        if (it == cache_loaded.end() || it->second.is_null()) {
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
        // Build blend tree if layer state was grown from late-join anim RPCs
        // while we were still a placeholder.
        if (anim_player && (!anim_tree || blend_tree.is_null()) && !anim_layers.empty())
            ensure_animation_layer(std::max(0, (int)anim_layers.size() - 1));
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
            if (!Tool::Format::is_supported_format(format_registry, Format::UNKNOWN, lp)) continue;
            try {
                const bool was_loaded = is_model_loaded(mn);
                const std::string before = was_loaded ? cache_hashes[mn] : "";
                load(mn, lp); // no-op if same hash; re-imports if bytes changed
                if (!was_loaded || cache_hashes[mn] != before)
                    loaded.push_back(mn);
                #if defined(VSDK_Client)
                Manager::Asset::get_singleton()->flush_spawn_queue(mn);
                #endif
            }
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
        #if defined(VSDK_Client)
        Manager::Asset::get_singleton()->flush_ready_spawns();
        #endif
    }

    void Model::sync_resource_models(const std::string& resource, const std::vector<std::string>& files) {
        // 1) Prune cache entries for this resource that are no longer in the
        //    authoritative file/model list (removed on server / updated meta).
        const std::string prefix = fmt::format(":{}/", resource);
        std::unordered_set<std::string> keep;
        keep.reserve(files.size());
        for (const auto& file : files) {
            if (!Tool::Format::is_supported_extension(format_registry, file)) continue;
            keep.insert(fmt::format(":{}/{}", resource, file));
        }
        std::vector<std::string> to_unload;
        for (const auto& [name, _] : cache_loaded) {
            if (name.rfind(prefix, 0) != 0) continue;
            if (!keep.count(name)) to_unload.push_back(name);
        }
        for (const auto& name : to_unload) {
            unload(name);
            #if defined(VSDK_Client)
            Manager::Asset::get_singleton()->clear_spawn_queue(name);
            #endif
        }
        if (!to_unload.empty()) {
            auto rm = Vital::Manager::Resource::get_singleton();
            rm->log("sbox", fmt::format("resource `{}` pruned {} stale model asset(s) from cache",
                resource, to_unload.size()));
        }
        // 2) Load anything missing (already-cached names are skipped inside).
        load_resource_models(resource, files);
    }

    void Model::unload_resource_models(const std::string& resource) {
        auto rm = Vital::Manager::Resource::get_singleton();
        const std::string prefix = fmt::format(":{}/", resource);
        std::vector<std::string> to_unload;
        for (const auto& [name, _] : cache_loaded) {
            if (name.rfind(prefix, 0) == 0) to_unload.push_back(name);
        }
        for (const auto& name : to_unload) {
            try { unload(name); } catch (...) {}
            #if defined(VSDK_Client)
            Manager::Asset::get_singleton()->clear_spawn_queue(name);
            #endif
        }
        #if defined(VSDK_Client)
        Manager::Asset::get_singleton()->clear_spawn_queue_prefix(prefix);
        #endif
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

        // If this model hasn't been network-registered yet, its _spawn_entity
        // RPC is still sitting on Core's deferred queue (see Model::create() —
        // registration is pushed a frame late so clients see _ready() side
        // effects first). Broadcasting _set_authority right now would race
        // ahead of that RPC: on every client, Network::_set_authority's
        // find_syncable(net_id) would find nothing yet (the model doesn't
        // exist there yet) and silently drop the update — permanently, since
        // it's never resent. Bail out here instead: sync_authority is already
        // updated above, and Model::create()'s deferred lambda reads it fresh
        // (via get_sync_authority()) when it finally sends _spawn_entity, so
        // that single RPC ends up carrying the correct authority for every
        // replica. This lets set_syncer() be called safely in the same Lua
        // tick as create(), with no need to pass authority into create() too.
        if (!sync_registered) return;

        // Already registered — a genuine authority reassignment (e.g.
        // possession swap) after the fact. Broadcast reliably to all clients
        // so each one knows whether to run interpolation or raw local physics.
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

    //--------------------------//
    //  Layered Anim Blending   //
    //--------------------------//
    //
    // Builds a small AnimationTree graph once per model:
    //
    //   anim_0 -> scale_0 ------------------------------\
    //   anim_1 -> scale_1 -> [blend_1] ------------------+-> output
    //   anim_2 -> scale_2 -> [blend_2] --/
    //   anim_3 -> scale_3 -> [blend_3] --/
    //
    // scale_N (AnimationNodeTimeScale) drives that layer's playback speed.
    // blend_N (AnimationNodeBlend2) fades that layer's contribution in/out
    // over layer weight, tweened smoothly in update_animation_layers().
    // Layer 0 is the base and always contributes at full weight.
    bool Model::ensure_animation_layer(int layer) {
        if (layer < 0 || layer >= ANIM_LAYER_SOFT_MAX) {
            godot::UtilityFunctions::push_warning(
                "Model::ensure_animation_layer — invalid layer index: ", layer,
                " (soft max ", ANIM_LAYER_SOFT_MAX, ")");
            return false;
        }
        const bool need_grow = (int)anim_layers.size() <= layer;
        if (need_grow) {
            const int old_count = (int)anim_layers.size();
            anim_layers.resize(layer + 1);
            // Layer 0 defaults to full weight once it exists.
            if (old_count == 0 && !anim_layers.empty()) {
                anim_layers[0].weight = 1.0f;
                anim_layers[0].weight_target = 1.0f;
            }
        }
        // Placeholder / pre-hydrate: layer slots may grow, tree builds later.
        if (!anim_player) return true;

        // Late-join path: layer state was sized while placeholder (anim_player
        // null), then hydrate set anim_player. Previous code returned early
        // because size > layer and never built anim_tree/blend_tree → null
        // deref in apply_play_animation_layer (second client crash).
        if (!need_grow && anim_tree && blend_tree.is_valid()) return true;

        // (Re)build graph when grown or tree missing after hydrate.
        {
            auto* player = anim_player;
            if (!anim_tree) {
                anim_tree = memnew(godot::AnimationTree);
                anim_tree->set_name(Tool::to_godot_string("@anim_blend_tree"));
                add_child(anim_tree);
                anim_tree->set_animation_player(anim_tree->get_path_to(player));
            }
            blend_tree = godot::Ref<godot::AnimationNodeBlendTree>(memnew(godot::AnimationNodeBlendTree));
            anim_tree->set_tree_root(blend_tree);

            const int count = (int)anim_layers.size();
            for (int i = 0; i < count; i++) {
                godot::Ref<godot::AnimationNodeAnimation> anim_node(memnew(godot::AnimationNodeAnimation));
                godot::Ref<godot::AnimationNodeTimeScale> scale_node(memnew(godot::AnimationNodeTimeScale));
                godot::String anim_name  = Tool::to_godot_string(fmt::format("anim_{}", i));
                godot::String scale_name = Tool::to_godot_string(fmt::format("scale_{}", i));
                blend_tree->add_node(anim_name, anim_node);
                blend_tree->add_node(scale_name, scale_node);
                blend_tree->connect_node(scale_name, 0, anim_name);
                // Restore assigned clip if any.
                if (!anim_layers[i].current_anim.empty())
                    anim_node->set_animation(Tool::to_godot_string(anim_layers[i].current_anim));

                // Restore direction + magnitude. A negative stored speed used
                // to get silently reset to 1.0 (forward) here whenever the
                // tree was rebuilt (e.g. the first time a higher layer index
                // came into use) — a layer that was mid-reverse would snap
                // back to forward with no warning. Direction now lives on
                // play_mode; TimeScale only ever gets a non-negative
                // magnitude, defaulting a truly-unset (zero) speed to 1.0.
                float restored_speed = anim_layers[i].speed;
                float restored_mag = restored_speed < 0.0f ? -restored_speed : restored_speed;
                if (restored_mag < 0.0001f) restored_mag = 1.0f;
                anim_node->set_play_mode(restored_speed < 0.0f
                    ? godot::AnimationNodeAnimation::PLAY_MODE_BACKWARD
                    : godot::AnimationNodeAnimation::PLAY_MODE_FORWARD);
                anim_tree->set(Tool::to_godot_string(fmt::format("parameters/scale_{}/scale", i)), restored_mag);
            }

            godot::String chain = Tool::to_godot_string("scale_0");
            for (int i = 1; i < count; i++) {
                godot::Ref<godot::AnimationNodeBlend2> blend_node(memnew(godot::AnimationNodeBlend2));
                godot::String blend_name = Tool::to_godot_string(fmt::format("blend_{}", i));
                godot::String scale_name = Tool::to_godot_string(fmt::format("scale_{}", i));
                blend_tree->add_node(blend_name, blend_node);
                blend_tree->connect_node(blend_name, 0, chain);
                blend_tree->connect_node(blend_name, 1, scale_name);
                anim_tree->set(Tool::to_godot_string(fmt::format("parameters/blend_{}/blend_amount", i)),
                    anim_layers[i].weight);
                chain = blend_name;
            }
            blend_tree->connect_node(Tool::to_godot_string("output"), 0, chain);

            if (!anim_layers.empty()) {
                anim_layers[0].weight = 1.0f;
                anim_layers[0].weight_target = 1.0f;
            }
            anim_tree->set_active(true);
            // Re-apply stored bone filters (tree nodes were recreated).
            for (int i = 1; i < count; i++) {
                if (anim_layers[i].filter_enabled)
                    apply_set_animation_layer_filter(i, true, anim_layers[i].filter_bones);
            }
        }
        return true;
    }

    void Model::build_animation_tree() {
        // Ensure at least the base layer exists, then let ensure_ build the graph.
        if (anim_layers.empty()) anim_layers.resize(1);
        ensure_animation_layer(0);
    }

    void Model::update_animation_layers(float delta) {
        for (int i = 1; i < (int)anim_layers.size(); i++) {
            auto& layer = anim_layers[i];

            // One-shot: when loop=false, auto-fade the layer after the clip ends
            // so Lua does not have to call stop_animation_layer manually.
            if (layer.one_shot && !layer.current_anim.empty() && layer.weight_target > 0.0f) {
                layer.one_shot_remaining -= delta;
                if (layer.one_shot_remaining <= 0.0f) {
                    layer.one_shot = false;
                    apply_stop_animation_layer(i, 0.25f);
                    layer.current_anim.clear();
                }
            }

            if (layer.weight == layer.weight_target) continue;

            float step = layer.weight_rate * delta;
            if (layer.weight < layer.weight_target) layer.weight = std::min(layer.weight_target, layer.weight + step);
            else layer.weight = std::max(layer.weight_target, layer.weight - step);

            if (anim_tree)
                anim_tree->set(Tool::to_godot_string(fmt::format("parameters/blend_{}/blend_amount", i)), layer.weight);
        }
    }

    int Model::get_animation_layer_count() const { return (int)anim_layers.size(); }

    // --- Pure local application — never broadcasts. Called both by the
    // public API (which broadcasts on top) and by Network::_sync_anim_layer
    // when mirroring a remote peer's animation state. ---

    bool Model::apply_play_animation_layer(int layer, const std::string& name, bool loop, float speed, float weight, float blend_time) {
        if (!ensure_animation_layer(layer)) {
            godot::UtilityFunctions::push_warning("Model::play_animation_layer — invalid layer index: ", layer);
            return false;
        }
        // Soft-fail: placeholder models / meshes without an AnimationPlayer must
        // not throw. Late-join _sync_anim_layer can arrive before hydrate().
        if (!anim_player) {
            godot::UtilityFunctions::push_warning(
                "Model::apply_play_animation_layer — no AnimationPlayer yet on '",
                Tool::to_godot_string(model_name), "' (placeholder=", placeholder, ")");
            return false;
        }
        auto* player = anim_player;
        if (!player->has_animation(Tool::to_godot_string(name))) {
            godot::UtilityFunctions::push_warning("Animation '", Tool::to_godot_string(name),
                "' not found in model '", Tool::to_godot_string(model_name), "'");
            return false;
        }
        build_animation_tree();
        if (!anim_tree || blend_tree.is_null()) {
            godot::UtilityFunctions::push_warning(
                "Model::apply_play_animation_layer — anim tree not ready on '",
                Tool::to_godot_string(model_name), "'");
            return false;
        }

        godot::Ref<godot::Animation> animation = player->get_animation(Tool::to_godot_string(name));
        if (animation.is_valid())
            animation->set_loop_mode(loop ? godot::Animation::LOOP_LINEAR : godot::Animation::LOOP_NONE);

        godot::String anim_key = Tool::to_godot_string(fmt::format("anim_{}", layer));
        bool is_one_shot_retrigger = !loop && layer > 0;

        // Direction now lives on AnimationNodeAnimation::play_mode rather
        // than on the sign of the TimeScale scale. PLAY_MODE_BACKWARD walks
        // the clip from its *end* toward frame 0 as the node's internal
        // clock advances from 0 — so a reversed one-shot (e.g. "wave" played
        // back-to-front) starts at the clip's last frame immediately, no
        // manual seek-to-end required. A negative TimeScale alone can't do
        // this: a one-shot node's internal position starts at 0, and a
        // negative delta from there just clamps at 0 and freezes. (Looping
        // clips don't have this problem since they wrap, but play_mode
        // handles both cases uniformly.) TimeScale itself always gets a
        // non-negative magnitude now; play_mode carries the sign.
        float speed_mag = speed < 0.0f ? -speed : speed;
        auto play_mode = speed < 0.0f
            ? godot::AnimationNodeAnimation::PLAY_MODE_BACKWARD
            : godot::AnimationNodeAnimation::PLAY_MODE_FORWARD;

        if (is_one_shot_retrigger) {
            // AnimationNodeAnimation keeps its own internal playback clock
            // tied to its position in the tree — re-assigning the same clip
            // name via set_animation() does NOT rewind it. Once a one-shot
            // (e.g. "wave") reaches its end it just freezes on the last
            // frame forever, so pressing Q again did nothing visible even
            // though play_animation_layer() was firing correctly. Recreating
            // the node gives it a fresh playback state every time it's
            // (re)triggered, so it always restarts from frame 0 (or, in
            // reverse, from the clip's last frame).
            blend_tree->remove_node(anim_key);
            godot::Ref<godot::AnimationNodeAnimation> fresh_node(memnew(godot::AnimationNodeAnimation));
            fresh_node->set_animation(Tool::to_godot_string(name));
            fresh_node->set_play_mode(play_mode);
            blend_tree->add_node(anim_key, fresh_node);
            blend_tree->connect_node(Tool::to_godot_string(fmt::format("scale_{}", layer)), 0, anim_key);
        } else {
            auto anim_node = godot::Object::cast_to<godot::AnimationNodeAnimation>(
                blend_tree->get_node(anim_key).ptr());
            if (anim_node) {
                anim_node->set_animation(Tool::to_godot_string(name));
                anim_node->set_play_mode(play_mode);
            }
        }
        anim_tree->set(Tool::to_godot_string(fmt::format("parameters/scale_{}/scale", layer)), speed_mag);

        auto& state = anim_layers[layer];
        state.current_anim = name;
        state.speed = speed;
        state.loop = loop;
        if (!loop && layer > 0 && animation.is_valid()) {
            float len = animation->get_length();
            float spd = speed_mag > 0.0001f ? speed_mag : 1.0f;
            state.one_shot = true;
            state.one_shot_remaining = len / spd;
        } else {
            state.one_shot = false;
            state.one_shot_remaining = 0.0f;
        }

        if (layer == 0) {
            // Base layer always contributes fully — nothing to tween.
            state.weight = 1.0f;
            state.weight_target = 1.0f;
        } else {
            weight = std::clamp(weight, 0.0f, 1.0f);
            state.weight_target = weight;
            state.weight_rate = blend_time > 0.0001f ? (1.0f / blend_time) : 1000.0f;
            if (blend_time <= 0.0001f) {
                state.weight = weight;
                anim_tree->set(Tool::to_godot_string(fmt::format("parameters/blend_{}/blend_amount", layer)), weight);
            }
        }
        return true;
    }

    void Model::apply_stop_animation_layer(int layer, float blend_time) {
        // Layer 0 is the always-on base — "stopping" it doesn't mean
        // anything (use play_animation_layer(0, ...) to switch its clip).
        if (layer <= 0 || layer >= (int)anim_layers.size() || !anim_tree) return;

        auto& state = anim_layers[layer];
        state.one_shot = false;
        state.one_shot_remaining = 0.0f;
        state.weight_target = 0.0f;
        state.weight_rate = blend_time > 0.0001f ? (1.0f / blend_time) : 1000.0f;
        if (blend_time <= 0.0001f) {
            state.weight = 0.0f;
            anim_tree->set(Tool::to_godot_string(fmt::format("parameters/blend_{}/blend_amount", layer)), 0.0f);
        }
    }

    bool Model::apply_set_animation_layer_weight(int layer, float weight, float blend_time) {
        if (layer <= 0 || layer >= (int)anim_layers.size() || !anim_tree) return false;

        weight = std::clamp(weight, 0.0f, 1.0f);
        auto& state = anim_layers[layer];
        state.weight_target = weight;
        state.weight_rate = blend_time > 0.0001f ? (1.0f / blend_time) : 1000.0f;
        if (blend_time <= 0.0001f) {
            state.weight = weight;
            anim_tree->set(Tool::to_godot_string(fmt::format("parameters/blend_{}/blend_amount", layer)), weight);
        }
        return true;
    }

    void Model::apply_set_animation_layer_speed(int layer, float speed) {
        if (layer < 0 || layer >= (int)anim_layers.size() || !anim_tree || blend_tree.is_null()) return;
        anim_layers[layer].speed = speed;

        // Same direction-via-play_mode / magnitude-via-TimeScale split as
        // apply_play_animation_layer, so flipping speed mid-playback (e.g.
        // a movement direction change) reverses correctly even for a
        // one-shot layer, not just a looping one.
        float speed_mag = speed < 0.0f ? -speed : speed;
        godot::String anim_key = Tool::to_godot_string(fmt::format("anim_{}", layer));
        auto anim_node = godot::Object::cast_to<godot::AnimationNodeAnimation>(
            blend_tree->get_node(anim_key).ptr());
        if (anim_node) {
            anim_node->set_play_mode(speed < 0.0f
                ? godot::AnimationNodeAnimation::PLAY_MODE_BACKWARD
                : godot::AnimationNodeAnimation::PLAY_MODE_FORWARD);
        }
        anim_tree->set(Tool::to_godot_string(fmt::format("parameters/scale_{}/scale", layer)), speed_mag);
    }

    // --- Network broadcast ---
    //
    // Server: always allowed to broadcast — it's authoritative by default.
    // Client: only the peer currently holding sync authority over this model
    // (see set_syncer()/sync_authority) may broadcast its own animation
    // state. Everyone else — including other clients rendering this model
    // in third person — receives and mirrors it via Network::_sync_anim_layer,
    // never drives it locally. Unreplicated models (net_id == 0, e.g. purely
    // decorative client-only props) never hit the network at all.
    //
    // mode: 0 = play, 1 = stop, 2 = set weight, 3 = set speed.
    void Model::broadcast_animation_layer(int mode, int layer, const std::string& name, bool loop, float speed, float weight, float blend_time) {
        if (net_id == 0) return;

        #if !defined(VSDK_Client)
        // Server: genuine broadcast to every connected client — same
        // pattern as broadcast_sync()/_wake_sync.
        auto* net_node = Manager::Network::get_singleton()->get_node();
        if (!net_node) return;
        net_node->rpc("_sync_anim_layer", (int)net_id, layer, mode,
            Tool::to_godot_string(name), loop, speed, weight, blend_time);
        #else
        // Client: only the peer holding sync authority over this model may
        // send its own animation state — and it must go to the server via
        // rpc_id(1, ...), never a bare rpc() broadcast. A client's only
        // transport connection is to the server; Godot's high-level
        // MultiplayerAPI does not implicitly resolve a client's broadcast
        // rpc() the way send_sync_to_server()/_sync_client rely on an
        // explicit rpc_id(1, ...) target — see that pair for the proven
        // pattern this mirrors. The server then relays it on to every
        // other client (see Network::_sync_anim_layer's server branch).
        auto* net_mgr = Manager::Network::get_singleton();
        if (!net_mgr || net_mgr->get_peer_id() != sync_authority) return;
        auto* net_node = net_mgr->get_node();
        if (!net_node) return;
        net_node->rpc_id(1, "_sync_anim_layer", (int)net_id, layer, mode,
            Tool::to_godot_string(name), loop, speed, weight, blend_time);
        #endif
    }

    void Model::apply_set_animation_layer_filter(int layer, bool enabled, const std::vector<std::string>& bone_paths) {
        // Layer 0 is the base chain — no Blend2 node (no filter target).
        if (layer < 1) return;
        if (!ensure_animation_layer(layer)) return;
        // Persist for rebuild + late-join dump (even if tree not ready yet).
        anim_layers[layer].filter_enabled = enabled && !bone_paths.empty();
        anim_layers[layer].filter_bones = enabled ? bone_paths : std::vector<std::string>{};

        // Ensure tree + skeleton discovery exist before resolving filter paths.
        if (!anim_player) return;
        if (!skeleton) find_node(this, skeleton);
        if (!anim_tree || blend_tree.is_null()) build_animation_tree();
        if (blend_tree.is_null()) return;

        auto node = blend_tree->get_node(Tool::to_godot_string(fmt::format("blend_{}", layer)));
        auto* blend = godot::Object::cast_to<godot::AnimationNodeBlend2>(node.ptr());
        if (!blend) return;

        // Reset so previous filter paths do not stick around.
        blend->set_filter_enabled(false);
        if (!enabled || bone_paths.empty()) return;

        // Godot AnimationMixer filter paths are relative to the player root and
        // for bones are almost always "NodePathToSkeleton:BoneName". Passing only
        // "DEF-hand.L" matches nothing → overlay contributes zero (invisible wave).
        godot::String skel_rel;
        if (skeleton && anim_player) {
            // Track/filter paths are resolved relative to the AnimationPlayer's
            // root_node (defaults to "..", i.e. its parent) — NOT relative to
            // the AnimationPlayer node itself. Using get_path_to() from the
            // player directly produces a path one level off (e.g. "../Rig/..."
            // instead of "Rig/..."), which never matches any real track path,
            // so the filter silently matches nothing and the whole overlay
            // layer contributes zero (no wave at all).
            godot::Node* root = anim_player->get_node_or_null(anim_player->get_root_node());
            if (!root) root = anim_player->get_parent();
            godot::NodePath p = root->get_path_to(skeleton);
            skel_rel = godot::String(p);
            if (skel_rel.begins_with("./")) skel_rel = skel_rel.substr(2);
        }

        blend->set_filter_enabled(true);
        for (const auto& raw : bone_paths) {
            godot::String bone = Tool::to_godot_string(raw);
            // Already a full filter path (contains ':') — use as-is.
            if (bone.contains(":")) {
                blend->set_filter_path(godot::NodePath(bone), true);
                continue;
            }
            // Bone name only — prefix with skeleton path from the AnimationPlayer.
            if (!skel_rel.is_empty()) {
                blend->set_filter_path(godot::NodePath(skel_rel + ":" + bone), true);
            }
            // Also register bare name as fallback (some GLBs author tracks that way).
            blend->set_filter_path(godot::NodePath(bone), true);
        }

        godot::UtilityFunctions::print(
            "Model::set_animation_layer_filter layer=", layer,
            " skeleton_path='", skel_rel, "' bones=", (int)bone_paths.size());
    }

    bool Model::set_animation_layer_filter(int layer, bool enabled, const std::vector<std::string>& bone_paths, bool sync) {
        if (layer < 1) return false;
        if (!ensure_animation_layer(layer)) return false;
        apply_set_animation_layer_filter(layer, enabled, bone_paths);
        if (sync) broadcast_animation_layer_filter(layer, enabled, bone_paths);
        return true;
    }

    void Model::broadcast_animation_layer_filter(int layer, bool enabled, const std::vector<std::string>& bone_paths) {
        if (net_id == 0) return;

        godot::PackedStringArray bones;
        for (const auto& b : bone_paths) bones.push_back(Tool::to_godot_string(b));

        #if !defined(VSDK_Client)
        auto* net_node = Manager::Network::get_singleton()->get_node();
        if (net_node)
            net_node->rpc("_sync_anim_layer_filter", (int)net_id, layer, enabled, bones);
        #else
        auto* net_mgr = Manager::Network::get_singleton();
        if (!net_mgr || net_mgr->get_peer_id() != sync_authority) return;
        auto* net_node = net_mgr->get_node();
        if (!net_node) return;
        net_node->rpc_id(1, "_sync_anim_layer_filter", (int)net_id, layer, enabled, bones);
        #endif
    }

    bool Model::play_animation_layer(int layer, const std::string& name, bool loop, float speed, float weight, float blend_time, bool sync) {
        if (!apply_play_animation_layer(layer, name, loop, speed, weight, blend_time)) return false;
        if (sync) broadcast_animation_layer(0, layer, name, loop, speed, weight, blend_time);
        return true;
    }

    void Model::stop_animation_layer(int layer, float blend_time, bool sync) {
        apply_stop_animation_layer(layer, blend_time);
        if (sync) broadcast_animation_layer(1, layer, "", true, 1.0f, 0.0f, blend_time);
    }

    bool Model::set_animation_layer_weight(int layer, float weight, float blend_time, bool sync) {
        if (!apply_set_animation_layer_weight(layer, weight, blend_time)) return false;
        if (sync) broadcast_animation_layer(2, layer, "", true, 1.0f, weight, blend_time);
        return true;
    }

    void Model::set_animation_layer_speed(int layer, float speed, bool sync) {
        apply_set_animation_layer_speed(layer, speed);
        if (sync) broadcast_animation_layer(3, layer, "", true, speed, 0.0f, 0.0f);
    }

    float Model::get_animation_layer_weight(int layer) const {
        if (layer < 0 || layer >= (int)anim_layers.size()) return 0.0f;
        return anim_layers[layer].weight_target;
    }

    float Model::get_animation_layer_speed(int layer) const {
        if (layer < 0 || layer >= (int)anim_layers.size()) return 0.0f;
        return anim_layers[layer].speed;
    }

    std::string Model::get_current_animation_layer(int layer) const {
        if (layer < 0 || layer >= (int)anim_layers.size()) return "";
        return anim_layers[layer].current_anim;
    }

    bool Model::is_animation_layer_playing(int layer) const {
        if (layer < 0 || layer >= (int)anim_layers.size()) return false;
        if (anim_layers[layer].current_anim.empty()) return false;
        return layer == 0 || anim_layers[layer].weight_target > 0.0f;
    }
}