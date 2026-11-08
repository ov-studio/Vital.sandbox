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
#include <Vital.sandbox/Engine/public/dimension.h>
#include <Vital.sandbox/Manager/public/asset.h>
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Manager/public/resource.h>


///////////////////////////
// Vital: Engine: Model //
///////////////////////////

// TODO: Improve
namespace Vital::Engine {
    // Asserts //
    godot::MeshInstance3D* Model::assert_component(const std::string& component) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("component '{}' not found in model '{}'", component, model_name));
        return mesh;
    }

    std::pair<godot::MeshInstance3D*, int> Model::assert_material(const std::string& component, const std::string& material) {
        auto mesh = assert_component(component);
        int idx = find_material_index(mesh, material);
        if (idx < 0) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("material '{}' not found in component '{}'", material, component));
        return { mesh, idx };
    }

    godot::Skeleton3D* Model::assert_skeleton() {
        if (!skeleton) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("no skeleton found in model '{}'", model_name));
        return skeleton;
    }

    void Model::assert_material_feature(int feature) {
        if (feature < 0 || feature >= godot::BaseMaterial3D::FEATURE_MAX) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "invalid material feature");
    }

    void Model::assert_material_flag(int flag) {
        if (flag < 0 || flag >= godot::BaseMaterial3D::FLAG_MAX) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "invalid material flag");
    }


    // Helpers //
    #if defined(VSDK_Client)
    godot::Array Model::build_lod_skin_pose(const godot::Ref<godot::GLTFState>& state, int skin_index) {
        godot::Array pose;
        auto skins = state->get_skins();
        if (skin_index < 0 || skin_index >= skins.size()) return pose;
        godot::Ref<godot::GLTFSkin> skin = skins[skin_index];
        if (skin.is_null()) return pose;

        auto nodes = state->get_nodes();
        const godot::PackedInt32Array joints = skin->get_joints_original();
        auto inverse_binds = skin->get_inverse_binds();
        const bool has_binds = inverse_binds.size() >= joints.size();

        std::unordered_map<int, godot::Transform3D> global_rest;
        auto get_global_rest = [&](int node_index) {
            std::vector<int> chain;
            godot::Transform3D base;
            int current = node_index;
            while (current >= 0 && current < nodes.size() && (int)chain.size() <= nodes.size()) {
                auto cached = global_rest.find(current);
                if (cached != global_rest.end()) { base = cached->second; break; }
                chain.push_back(current);
                godot::Ref<godot::GLTFNode> node = nodes[current];
                if (node.is_null()) break;
                int parent = node->get_parent();
                if (parent < 0 || parent >= nodes.size()) break;
                godot::Ref<godot::GLTFNode> parent_node = nodes[parent];
                if (parent_node.is_null() || parent_node->get_skeleton() != node->get_skeleton()) break;
                current = parent;
            }
            for (int i = (int)chain.size() - 1; i >= 0; i--) {
                godot::Ref<godot::GLTFNode> node = nodes[chain[i]];
                godot::Transform3D local = node.is_valid() ? node->get_xform() : godot::Transform3D();
                local.basis = godot::Basis(local.basis.get_rotation_quaternion(), local.basis.get_scale());
                base = base * local;
                global_rest[chain[i]] = base;
            }
            return base;
        };

        for (int i = 0; i < joints.size(); i++) {
            godot::Transform3D bind;
            if (has_binds) bind = inverse_binds[i];
            const int node_index = joints[i];
            const bool valid_node = node_index >= 0 && node_index < nodes.size();
            pose.push_back((valid_node ? get_global_rest(node_index) : godot::Transform3D()) * bind);
        }
        return pose;
    }

    godot::Ref<godot::ShaderMaterial> Model::get_wireframe_material() {
        if (!wireframe_material.is_valid()) {
            godot::Ref<godot::Shader> shader;
            shader.instantiate();
            shader -> set_code(
                "shader_type spatial;\n"
                "render_mode unshaded, wireframe, cull_disabled;\n"
                "uniform vec4 color : source_color = vec4(0.0, 1.0, 0.4, 1.0);\n"
                "void fragment() { ALBEDO = color.rgb; }\n"
            );
            wireframe_material.instantiate();
            wireframe_material -> set_shader(shader);
        }
        return wireframe_material;
    }
    #endif

    godot::MeshInstance3D* Model::find_component(const std::string& component) {
        if (auto mesh = find_mesh_node(this, component)) return mesh;
        auto it = detached_components.find(component);
        return it != detached_components.end() ? it->second : nullptr;
    }

    int Model::find_material_index(godot::MeshInstance3D* mesh, const std::string& material) {
        godot::ArrayMesh* array_mesh = godot::Object::cast_to<godot::ArrayMesh>(mesh -> get_mesh().ptr());
        if (!array_mesh) return -1;
        for (int i = 0; i < array_mesh->get_surface_count(); i++) {
            if (Tool::to_std_string(array_mesh->surface_get_name(i)) == material) return i;
        }
        return -1;
    }

    bool Model::ensure_animation_layer(int layer) {
        if (layer < 0 || layer >= ANIM_LAYER_SOFT_MAX) {
            godot::UtilityFunctions::push_warning("Model::ensure_animation_layer — invalid layer index: ", layer, " (soft max ", ANIM_LAYER_SOFT_MAX, ")");
            return false;
        }
        const bool need_grow = (int)anim_layers.size() <= layer;
        if (need_grow) {
            const int old_count = (int)anim_layers.size();
            anim_layers.resize(layer + 1);
            if (old_count == 0 && !anim_layers.empty()) {
                anim_layers[0].weight = 1.0f;
                anim_layers[0].weight_target = 1.0f;
            }
        }
        if (!anim_player) return true;

        if (!need_grow && anim_tree && blend_tree.is_valid()) return true;

        {
            auto player = anim_player;
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
                if (!anim_layers[i].current_anim.empty()) anim_node->set_animation(Tool::to_godot_string(anim_layers[i].current_anim));

                float restored_speed = anim_layers[i].speed;
                float restored_mag = restored_speed < 0.0f ? -restored_speed : restored_speed;
                if (restored_mag < 0.0001f) restored_mag = 1.0f;
                anim_node->set_play_mode(restored_speed < 0.0f ? godot::AnimationNodeAnimation::PLAY_MODE_BACKWARD : godot::AnimationNodeAnimation::PLAY_MODE_FORWARD);
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
                anim_tree->set(Tool::to_godot_string(fmt::format("parameters/blend_{}/blend_amount", i)), anim_layers[i].weight);
                chain = blend_name;
            }
            blend_tree->connect_node(Tool::to_godot_string("output"), 0, chain);

            if (!anim_layers.empty()) {
                anim_layers[0].weight = 1.0f;
                anim_layers[0].weight_target = 1.0f;
            }
            anim_tree->set_active(true);
            for (int i = 1; i < count; i++) {
                if (anim_layers[i].filter_enabled) apply_set_animation_layer_filter(i, true, anim_layers[i].filter_bones);
            }
        }
        return true;
    }

    void Model::build_animation_tree() {
        if (anim_layers.empty()) anim_layers.resize(1);
        ensure_animation_layer(0);
    }

    void Model::update_animation_layers(float delta) {
        for (int i = 1; i < (int)anim_layers.size(); i++) {
            auto& layer = anim_layers[i];
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
            if (anim_tree) anim_tree->set(Tool::to_godot_string(fmt::format("parameters/blend_{}/blend_amount", i)), layer.weight);
        }
    }

    bool Model::apply_play_animation_layer(int layer, const std::string& name, bool loop, float speed, float weight, float blend_time) {
        if (!ensure_animation_layer(layer)) {
            godot::UtilityFunctions::push_warning("Model::play_animation_layer — invalid layer index: ", layer);
            return false;
        }
        if (!anim_player) {
            godot::UtilityFunctions::push_warning("Model::apply_play_animation_layer — no AnimationPlayer yet on '", Tool::to_godot_string(model_name), "' (placeholder=", placeholder, ")");
            return false;
        }
        auto player = anim_player;
        if (!player->has_animation(Tool::to_godot_string(name))) {
            godot::UtilityFunctions::push_warning("Animation '", Tool::to_godot_string(name), "' not found in model '", Tool::to_godot_string(model_name), "'");
            return false;
        }
        build_animation_tree();
        if (!anim_tree || blend_tree.is_null()) {
            godot::UtilityFunctions::push_warning("Model::apply_play_animation_layer — anim tree not ready on '", Tool::to_godot_string(model_name), "'");
            return false;
        }

        godot::Ref<godot::Animation> animation = player->get_animation(Tool::to_godot_string(name));
        if (animation.is_valid()) animation->set_loop_mode(loop ? godot::Animation::LOOP_LINEAR : godot::Animation::LOOP_NONE);
        godot::String anim_key = Tool::to_godot_string(fmt::format("anim_{}", layer));
        bool is_one_shot_retrigger = !loop && layer > 0;

        float speed_mag = speed < 0.0f ? -speed : speed;
        auto play_mode = speed < 0.0f ? godot::AnimationNodeAnimation::PLAY_MODE_BACKWARD : godot::AnimationNodeAnimation::PLAY_MODE_FORWARD;
        if (is_one_shot_retrigger) {
            blend_tree->remove_node(anim_key);
            godot::Ref<godot::AnimationNodeAnimation> fresh_node(memnew(godot::AnimationNodeAnimation));
            fresh_node->set_animation(Tool::to_godot_string(name));
            fresh_node->set_play_mode(play_mode);
            blend_tree->add_node(anim_key, fresh_node);
            blend_tree->connect_node(Tool::to_godot_string(fmt::format("scale_{}", layer)), 0, anim_key);
        }
        else {
            auto anim_node = godot::Object::cast_to<godot::AnimationNodeAnimation>(blend_tree->get_node(anim_key).ptr());
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
        }
        else {
            state.one_shot = false;
            state.one_shot_remaining = 0.0f;
        }

        if (layer == 0) {
            state.weight = 1.0f;
            state.weight_target = 1.0f;
        }
        else {
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
        float speed_mag = speed < 0.0f ? -speed : speed;
        godot::String anim_key = Tool::to_godot_string(fmt::format("anim_{}", layer));
        auto anim_node = godot::Object::cast_to<godot::AnimationNodeAnimation>(blend_tree->get_node(anim_key).ptr());
        if (anim_node) anim_node->set_play_mode(speed < 0.0f ? godot::AnimationNodeAnimation::PLAY_MODE_BACKWARD : godot::AnimationNodeAnimation::PLAY_MODE_FORWARD);
        anim_tree->set(Tool::to_godot_string(fmt::format("parameters/scale_{}/scale", layer)), speed_mag);
    }

    void Model::apply_set_animation_layer_filter(int layer, bool enabled, const std::vector<std::string>& bone_paths) {
        if (layer < 1) return;
        if (!ensure_animation_layer(layer)) return;
        anim_layers[layer].filter_enabled = enabled && !bone_paths.empty();
        anim_layers[layer].filter_bones = enabled ? bone_paths : std::vector<std::string>{};

        if (!anim_player) return;
        if (!skeleton) find_node(this, skeleton);
        if (!anim_tree || blend_tree.is_null()) build_animation_tree();
        if (blend_tree.is_null()) return;

        auto node = blend_tree->get_node(Tool::to_godot_string(fmt::format("blend_{}", layer)));
        auto blend = godot::Object::cast_to<godot::AnimationNodeBlend2>(node.ptr());
        if (!blend) return;

        blend->set_filter_enabled(false);
        if (!enabled || bone_paths.empty()) return;

        godot::String skel_rel;
        if (skeleton && anim_player) {
            godot::Node* root = anim_player->get_node_or_null(anim_player->get_root_node());
            if (!root) root = anim_player->get_parent();
            godot::NodePath p = root->get_path_to(skeleton);
            skel_rel = godot::String(p);
            if (skel_rel.begins_with("./")) skel_rel = skel_rel.substr(2);
        }

        blend->set_filter_enabled(true);
        for (const auto& raw : bone_paths) {
            godot::String bone = Tool::to_godot_string(raw);
            if (bone.contains(":")) {
                blend->set_filter_path(godot::NodePath(bone), true);
                continue;
            }
            if (!skel_rel.is_empty()) blend->set_filter_path(godot::NodePath(skel_rel + ":" + bone), true);
            blend->set_filter_path(godot::NodePath(bone), true);
        }
    }

    void Model::broadcast_animation_layer(int mode, int layer, const std::string& name, bool loop, float speed, float weight, float blend_time) {
        if (net_id == 0) return;

        #if !defined(VSDK_Client)
        Manager::Network::rpc(true, "_sync_anim_layer", (int)net_id, layer, mode, Tool::to_godot_string(name), loop, speed, weight, blend_time);
        #else
        auto net_mgr = Manager::Network::get_singleton();
        if (!net_mgr || net_mgr->get_peer_id() != sync_authority) return;
        Manager::Network::rpc_id(false, 1, "_sync_anim_layer", (int)net_id, layer, mode, Tool::to_godot_string(name), loop, speed, weight, blend_time);
        #endif
    }

    void Model::broadcast_animation_layer_filter(int layer, bool enabled, const std::vector<std::string>& bone_paths) {
        if (net_id == 0) return;

        godot::PackedStringArray bones;
        for (const auto& b : bone_paths) bones.push_back(Tool::to_godot_string(b));

        #if !defined(VSDK_Client)
        Manager::Network::rpc(true, "_sync_anim_layer_filter", (int)net_id, layer, enabled, bones);
        #else
        auto net_mgr = Manager::Network::get_singleton();
        if (!net_mgr || net_mgr->get_peer_id() != sync_authority) return;
        Manager::Network::rpc_id(false, 1, "_sync_anim_layer_filter", (int)net_id, layer, enabled, bones);
        #endif
    }


    // Hooks //
    void Model::_ready() {
        if (placeholder) return;
        find_node(this, skeleton);
        find_node(this, anim_player);
        sync_authority = pending_authority;
        sync_last_pos = get_global_position();
        sync_last_rot = get_rotation_degrees();
        sync_sleeping = false;
        sync_accum = 0.0f;
        reset_sync_state();
        #if defined(VSDK_Client)
        if (wireframe_all) set_component_wireframe("*", true);
        #endif
    }

    void Model::_notification(int what) {
        if (what == NOTIFICATION_PREDELETE) {
            for (auto& [name, mesh] : detached_components) {
                if (mesh) mesh->queue_free();
            }
            detached_components.clear();

            Tool::Event::emit("entity:unspawn", Tool::Stack({this}));
            Manager::Network::get_singleton()->unregister_syncable(this);
            sync_registered = false;
        }
    }

    void Model::_process(double delta) {
        if (anim_tree) update_animation_layers((float)delta);
        on_sync_process(delta);
    }


    // ISyncable interface //
    bool Model::is_sync_active() const {
        return const_cast<Model*>(this)->is_inside_tree() && !placeholder;
    }

    void Model::apply_sync(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3 scale) {
        if (!is_inside_tree()) return;

        auto net = Manager::Network::get_singleton();
        if (net && net->get_peer_id() == sync_authority) return;

        sync_push_snapshot(pos, rot, vel);
        set_scale(scale);
        sync_last_pos = pos;
        sync_last_rot = rot;
        sync_last_vel = vel;
        sync_last_scale = scale;
        sync_sleeping = false;
    }


    // Managers //
    bool Model::load(const std::string& name, const std::string& base, const std::string& path) {
        const std::string file_hash = Tool::File::hash(base, path);
        if (is_model_loaded(name)) {
            auto hit = cache_hashes.find(name);
            if (hit != cache_hashes.end() && hit->second == file_hash) return true;
            unload(name);
        }
        if (!load_from_buffer(name, Tool::File::read_binary(base, path))) return false;
        cache_hashes[name] = file_hash;
        return true;
    }

    bool Model::load_from_buffer(const std::string& name, const godot::PackedByteArray& buffer) {
        static std::mutex load_mutex;
        std::lock_guard<std::mutex> load_lock(load_mutex);
        if (is_model_loaded(name)) return true;

        godot::Ref<godot::PackedScene> scene;
        switch (Tool::Format::get_format(format_registry, Format::UNKNOWN, buffer)) {
            case Format::GLB: {
                godot::Ref<godot::GLTFDocument> document = memnew(godot::GLTFDocument);
                godot::Ref<godot::GLTFState>    state    = memnew(godot::GLTFState);
                #if !defined(VSDK_Client)
                state->set("handle_binary_image_mode", 0);
                #endif
                if (document->append_from_buffer(buffer, "", state) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "invalid model buffer");
                #if defined(VSDK_Client)
                {
                    std::unordered_map<int, int> mesh_skin;
                    auto gltf_nodes = state->get_nodes();
                    for (int i = 0; i < gltf_nodes.size(); i++) {
                        godot::Ref<godot::GLTFNode> gltf_node = gltf_nodes[i];
                        if (gltf_node.is_null()) continue;
                        const int mesh_index = gltf_node->get_mesh(), skin_index = gltf_node->get_skin();
                        if (mesh_index >= 0 && skin_index >= 0) mesh_skin.emplace(mesh_index, skin_index);
                    }
                    std::unordered_map<int, godot::Array> skin_pose_cache;
                    auto gltf_meshes = state->get_meshes();
                    for (int i = 0; i < gltf_meshes.size(); i++) {
                        godot::Ref<godot::GLTFMesh> gltf_mesh = gltf_meshes[i];
                        if (gltf_mesh.is_null()) continue;
                        godot::Ref<godot::ImporterMesh> importer_mesh = gltf_mesh->get_mesh();
                        if (importer_mesh.is_null()) continue;
                        godot::Array skin_pose;
                        auto skinned = mesh_skin.find(i);
                        if (skinned != mesh_skin.end()) {
                            auto cached = skin_pose_cache.find(skinned->second);
                            if (cached == skin_pose_cache.end()) cached = skin_pose_cache.emplace(skinned->second, build_lod_skin_pose(state, skinned->second)).first;
                            skin_pose = cached->second;
                        }
                        importer_mesh->generate_lods(LOD_NORMAL_MERGE_ANGLE, LOD_NORMAL_SPLIT_ANGLE, skin_pose);
                    }
                }
                #endif
                godot::Node* root = document->generate_scene(state);
                if (!root) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "failed to generate scene");
                scene = godot::Ref<godot::PackedScene>(memnew(godot::PackedScene));
                if (scene->pack(root) != godot::OK) {
                    memdelete(root);
                    throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "failed to pack model scene");
                }
                memdelete(root);
                document.unref();
                state.unref();
                break;
            }
            default: break;
        }

        if (scene.is_null()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "unsupported or invalid model format");
        cache_loaded[name] = scene;
        #if defined(VSDK_Client)
        Manager::Asset::get_singleton()->flush_spawn_queue(name);
        Engine::Core::get_singleton()->enqueue([name]() {
            auto it = load_waiters.find(name);
            if (it == load_waiters.end()) return;
            auto waiting = std::move(it->second);
            load_waiters.erase(it);
            for (auto& exec : waiting) exec();
        });
        #endif
        return true;
    }

    bool Model::unload(const std::string& name) {
        auto it = cache_loaded.find(name);
        if (it == cache_loaded.end()) return false;
        cache_loaded.erase(it);
        cache_hashes.erase(name);
        load_waiters.erase(name);
        for (auto sit = shape_cache.begin(); sit != shape_cache.end(); ) {
            if (sit -> first.compare(0, name.size() + 1, name + "|") == 0) sit = shape_cache.erase(sit);
            else ++sit;
        }
        return true;
    }

    void Model::when_loaded(const std::string& name, std::function<void()> exec) {
        if (is_model_loaded(name)) { exec(); return; }
        load_waiters[name].push_back(std::move(exec));
    }

    Model* Model::create(const std::string& name) {
        auto it = cache_loaded.find(name);
        if (it == cache_loaded.end()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("model '{}' isn't loaded yet", name));

        #if defined(VSDK_Client)
            Model* object = memnew(Model);
            object->set_model_name(name);
            godot::Node* instance = it->second->instantiate();
            if (!instance) {
                memdelete(object);
                throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("failed to instantiate model '{}'", name));
            }
            object->add_child(instance);
            Engine::Core::get_singleton()->add_child(object);
            Tool::Event::emit("entity:spawn", Tool::Stack({object, false}));
            return object;
        #else
            Model* object = memnew(Model);
            object->net_id = next_net_id++;

            godot::Node* instance = it->second->instantiate();
            if (!instance) {
                memdelete(object);
                throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("failed to instantiate model '{}'", name));
            }
            object->set_model_name(name);

            object->add_child(instance);
            Engine::Core::get_singleton()->add_child(object);

            uint32_t captured_net_id = object->net_id;
            godot::String captured_name = Tool::to_godot_string(name);
            godot::ObjectID captured_oid = godot::ObjectID(object->get_instance_id());
            Engine::Core::get_singleton()->enqueue([captured_oid, captured_net_id, captured_name]() {
                godot::Object* obj = godot::ObjectDB::get_instance(captured_oid);
                if (!obj) return;
                auto object = godot::Object::cast_to<Model>(obj);
                if (!object) return;

                Manager::Network::get_singleton()->enqueue_syncable_registration(object);
                {
                    godot::Vector3 spawn_pos = object->get_sync_position();
                    godot::Vector3 spawn_rot = object->get_sync_rotation();
                    Manager::Network::rpc(false, "_spawn_entity", (int)captured_net_id, (int)Engine::ISyncable::Type::Model, captured_name, object->get_sync_authority(), spawn_pos, spawn_rot);
                }
                object->flush_pending_force_transform();
                Tool::Event::emit("entity:spawn", Tool::Stack({object, false}));
            });
            return object;
        #endif
    }

    void Model::destroy() {
        #if !defined(VSDK_Client)
        if (net_id != 0) {
            Manager::Network::rpc(true, "_destroy_entity", (int)net_id);
        }
        #endif
        this->queue_free();
    }

    #if defined(VSDK_Client)
    void Model::hydrate(int authority_peer) {
        if (!placeholder) return;
        auto it = cache_loaded.find(model_name);
        if (it == cache_loaded.end() || it->second.is_null()) {
            godot::UtilityFunctions::push_warning("Model::hydrate — model not in cache: ", Tool::to_godot_string(model_name));
            return;
        }
        godot::Node* instance = it->second->instantiate();
        if (!instance) {
            godot::UtilityFunctions::push_warning("Model::hydrate — failed to instantiate: ", Tool::to_godot_string(model_name));
            return;
        }

        placeholder = false;
        pending_authority = authority_peer;
        add_child(instance);
        find_node(this, skeleton);
        find_node(this, anim_player);
        if (anim_player && (!anim_tree || blend_tree.is_null()) && !anim_layers.empty()) ensure_animation_layer(std::max(0, (int)anim_layers.size() - 1));

        for (const auto& name : component_hidden) {
            godot::MeshInstance3D* mesh = find_mesh_node(this, name);
            if (mesh) mesh->set_visible(false);
        }
        for (const auto& name : component_detached) {
            if (detached_components.count(name)) continue;
            godot::MeshInstance3D* mesh = find_mesh_node(this, name);
            if (!mesh) continue;
            godot::Node* parent = mesh->get_parent();
            if (!parent) continue;
            mesh->set_physics_process(false);
            mesh->set_process(false);
            parent->remove_child(mesh);
            detached_components[name] = mesh;
        }

        for (const auto& [name, transform] : component_transforms) {
            godot::MeshInstance3D* mesh = find_component(name);
            if (mesh) mesh->set_transform(transform);
        }

        for (const auto& [component, shapes] : blendshape_values) {
            godot::MeshInstance3D* mesh = find_component(component);
            if (!mesh) continue;
            for (const auto& [name, value] : shapes) {
                int idx = mesh->find_blend_shape_by_name(Tool::to_godot_string(name));
                if (idx >= 0) mesh->set_blend_shape_value(idx, value);
            }
        }

        if (wireframe_all) set_component_wireframe("*", true);

        sync_authority = authority_peer;
        sync_last_pos  = get_global_position();
        sync_last_rot  = get_rotation_degrees();
        if (!sync_registered) Manager::Network::get_singleton()->enqueue_syncable_registration(this);
        Engine::Dimension::set_node_visible(this, true);
        Tool::Event::emit("entity:spawn", Tool::Stack({this, true}));
    }
    #endif

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
        cache_loaded.clear();
        cache_hashes.clear();
        shape_cache.clear();
        load_waiters.clear();
        next_net_id = 1;
    }

    std::vector<std::string> Model::filter_resource_models(const std::string& resource, const std::vector<std::string>& files) {
        std::vector<std::string> validated;
        for (const auto& file : files) {
            if (!Tool::Format::is_supported_extension(format_registry, file)) continue;
            if (!Tool::Format::is_supported_format(format_registry, Format::UNKNOWN, fmt::format("{}/{}/{}", Vital::Manager::Resource::get_resources_root(), resource, file))) continue;
            validated.push_back(file);
        }
        return validated;
    }

    void Model::load_resource_models(const std::string& resource, const std::vector<std::string>& files) {
        auto rm = Vital::Manager::Resource::get_singleton();
        const std::string base = Vital::Manager::Resource::get_resource_base(resource);
        std::vector<std::string> loaded, failed;
        for (const auto& file : files) {
            if (!Tool::Format::is_supported_extension(format_registry, file)) continue;
            const std::string mn = fmt::format(":{}/{}", resource, file);
            if (!Tool::Format::is_supported_format(format_registry, Format::UNKNOWN, fmt::format("{}/{}/{}", Vital::Manager::Resource::get_resources_root(), resource, file))) continue;
            try {
                const bool was_loaded = is_model_loaded(mn);
                const std::string before = was_loaded ? cache_hashes[mn] : "";
                load(mn, base, file);
                if (!was_loaded || cache_hashes[mn] != before) loaded.push_back(mn);
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
            rm->log("sbox", fmt::format("resource `{}` pruned {} stale model asset(s) from cache", resource, to_unload.size()));
        }
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
        if (!to_unload.empty()) rm->log("sbox", fmt::format("resource `{}` unloaded {} model asset(s)", resource, to_unload.size()));
    }


    // Checkers //
    bool Model::is_model_loaded(const std::string& name) {
        return cache_loaded.find(name) != cache_loaded.end();
    }

    bool Model::is_component_visible(const std::string& component) {
        return assert_component(component)->is_visible();
    }

    bool Model::is_component_rendered(const std::string& component) {
        if (Tool::contains_wildcard(component)) {
            for (const auto& name : get_components())
                if (Tool::match_wildcard(component, name) && !detached_components.count(name)) return true;
            return false;
        }
        return detached_components.count(component) == 0;
    }

    #if defined(VSDK_Client)
    bool Model::is_component_wireframe(const std::string& component) {
        godot::MeshInstance3D* mesh = find_component(component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("component '{}' not found in model '{}'", component, model_name));
        return mesh -> get_material_overlay().ptr() == get_wireframe_material().ptr();
    }

    bool Model::is_wireframe_all() {
        return wireframe_all;
    }
    #endif

    bool Model::is_material_visible(const std::string& component, const std::string& material) {
        auto [mesh, idx] = assert_material(component, material);
        return !mesh->get_surface_override_material(idx).is_valid();
    }

    bool Model::is_material_feature(const std::string& component, const std::string& material, int feature) {
        assert_material_feature(feature);
        auto [mesh, idx] = assert_material(component, material);
        godot::Ref<godot::StandardMaterial3D> std_mat = godot::Object::cast_to<godot::StandardMaterial3D>(mesh->get_active_material(idx).ptr());
        if (!std_mat.is_valid()) return false;
        return std_mat->get_feature(static_cast<godot::BaseMaterial3D::Feature>(feature));
    }

    bool Model::is_material_flag(const std::string& component, const std::string& material, int flag) {
        assert_material_flag(flag);
        auto [mesh, idx] = assert_material(component, material);
        godot::Ref<godot::StandardMaterial3D> std_mat = godot::Object::cast_to<godot::StandardMaterial3D>(mesh->get_active_material(idx).ptr());
        if (!std_mat.is_valid()) return false;
        return std_mat->get_flag(static_cast<godot::BaseMaterial3D::Flags>(flag));
    }

    bool Model::is_animation_layer_playing(int layer) const {
        if (layer < 0 || layer >= (int)anim_layers.size()) return false;
        if (anim_layers[layer].current_anim.empty()) return false;
        return layer == 0 || anim_layers[layer].weight_target > 0.0f;
    }


    // Getters //
    godot::Ref<godot::PackedScene> Model::get_scene(const std::string& name) {
        auto it = cache_loaded.find(name);
        return it == cache_loaded.end() ? godot::Ref<godot::PackedScene>() : it -> second;
    }

    std::string Model::get_model_name() {
        return model_name;
    }

    uint32_t Model::get_net_id() const {
        return net_id;
    }

    std::vector<std::string> Model::get_components(bool include_all) {
        std::vector<std::string> components;
        collect_mesh_nodes(this, components, "");
        if (include_all) {
            for (const auto& [name, _] : detached_components) components.push_back(name);
        }
        return components;
    }

    float Model::get_component_lod_bias(const std::string& component) {
        godot::MeshInstance3D* mesh = find_component(component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("component '{}' not found in model '{}'", component, model_name));
        return mesh->get_lod_bias();
    }

    std::vector<std::string> Model::get_materials(const std::string& component) {
        godot::ArrayMesh* array_mesh =
            godot::Object::cast_to<godot::ArrayMesh>(assert_component(component) -> get_mesh().ptr());
        std::vector<std::string> materials;
        if (!array_mesh) return materials;
        for (int i = 0; i < array_mesh->get_surface_count(); i++) materials.push_back(Tool::to_std_string(array_mesh->surface_get_name(i)));
        return materials;
    }

    std::vector<std::string> Model::get_blendshapes(const std::string& component) {
        auto mesh = assert_component(component);
        godot::ArrayMesh* array_mesh = godot::Object::cast_to<godot::ArrayMesh>(mesh -> get_mesh().ptr());
        std::vector<std::string> blendshapes;
        if (!array_mesh) return blendshapes;
        for (int i = 0; i < mesh->get_blend_shape_count(); i++) blendshapes.push_back(Tool::to_std_string(array_mesh->get_blend_shape_name(i)));
        return blendshapes;
    }

    std::vector<std::string> Model::get_bones() {
        std::vector<std::string> bones;
        if (skeleton) for (int i = 0; i < skeleton->get_bone_count(); i++) bones.push_back(Tool::to_std_string(skeleton->get_bone_name(i)));
        return bones;
    }

    std::vector<std::string> Model::get_animations() {
        std::vector<std::string> animations;
        if (anim_player) {
            auto list = anim_player->get_animation_list();
            for (int i = 0; i < list.size(); i++) animations.push_back(Tool::to_std_string(list[i]));
        }
        return animations;
    }

    float Model::get_blendshape_value(const std::string& component, const std::string& blend_shape) {
        auto mesh = assert_component(component);
        int idx = mesh->find_blend_shape_by_name(Tool::to_godot_string(blend_shape));
        if (idx < 0) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("blendshape '{}' not found in component '{}'", blend_shape, component));
        return mesh->get_blend_shape_value(idx);
    }

    godot::Vector3 Model::get_bone_position(const std::string& bone) {
        auto skel = assert_skeleton();
        int idx = skel->find_bone(Tool::to_godot_string(bone));
        if (idx == -1) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("bone '{}' not found in model '{}'", bone, model_name));
        return skel->get_global_transform().xform(skel->get_bone_global_pose(idx).origin);
    }

    std::string Model::get_current_animation_layer(int layer) const {
        if (layer < 0 || layer >= (int)anim_layers.size()) return "";
        return anim_layers[layer].current_anim;
    }

    float Model::get_animation_layer_speed(int layer) const {
        if (layer < 0 || layer >= (int)anim_layers.size()) return 0.0f;
        return anim_layers[layer].speed;
    }

    float Model::get_animation_layer_weight(int layer) const {
        if (layer < 0 || layer >= (int)anim_layers.size()) return 0.0f;
        return anim_layers[layer].weight_target;
    }

    int Model::get_animation_layer_count() const {
        return (int)anim_layers.size();
    }


    // Setters //
    void Model::set_model_name(const std::string& name) {
        model_name = name;
    }

    bool Model::set_component_visible(const std::string& component, bool state, bool sync) {
        bool broadcast = false;
        auto exec = [&](const std::string& name) -> bool {
            godot::MeshInstance3D* mesh = find_mesh_node(this, name);
            if (!mesh) return false;
            bool changed = (component_hidden.count(name) > 0) == state;
            if (needs_broadcast(fmt::format("visible:{}", name), changed, sync)) broadcast = true;
            mesh->set_visible(state);
            if (!state) component_hidden.insert(name);
            else        component_hidden.erase(name);
            return true;
        };
        if (!apply_wildcard(component, [&]{ return get_components(); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("component '{}' not found in model '{}'", component, model_name));
        #if !defined(VSDK_Client)
        if (broadcast) broadcast_component_visible(component, state);
        #endif
        return true;
    }

    bool Model::set_component_rendered(const std::string& component, bool state, bool sync) {
        bool broadcast = false;
        if (state) {
            auto exec = [&](const std::string& name) -> bool {
                auto it = detached_components.find(name);
                if (it == detached_components.end()) return true;
                godot::MeshInstance3D* mesh = it->second;
                detached_components.erase(it);
                component_detached.erase(name);
                if (needs_broadcast(fmt::format("rendered:{}", name), true, sync)) broadcast = true;
                this->add_child(mesh);
                mesh->set_physics_process(true);
                mesh->set_process(true);
                return true;
            };
            if (Tool::contains_wildcard(component)) {
                std::vector<std::string> keys;
                keys.reserve(detached_components.size());
                for (const auto& [name, _] : detached_components) keys.push_back(name);
                for (const auto& name : keys)
                    if (Tool::match_wildcard(component, name)) exec(name);
            }
            else {
                if (!detached_components.count(component) && !find_mesh_node(this, component))
                    throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("component '{}' not found in model '{}'", component, model_name));
                exec(component);
            }
        }
        else {
            auto live_names = get_components();
            auto exec = [&](const std::string& name) -> bool {
                if (detached_components.count(name)) return true;
                godot::MeshInstance3D* mesh = find_mesh_node(this, name);
                if (!mesh) return false;
                godot::Node* parent = mesh->get_parent();
                if (!parent) return false;
                mesh->set_physics_process(false);
                mesh->set_process(false);
                parent->remove_child(mesh);
                detached_components[name] = mesh;
                component_detached.insert(name);
                if (needs_broadcast(fmt::format("rendered:{}", name), true, sync)) broadcast = true;
                return true;
            };
            if (Tool::contains_wildcard(component)) {
                for (const auto& name : live_names)
                    if (Tool::match_wildcard(component, name)) exec(name);
            }
            else {
                if (!exec(component))
                    throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("component '{}' not found in model '{}'", component, model_name));
            }
        }

        #if !defined(VSDK_Client)
        if (broadcast) broadcast_component_rendered(component, state);
        #endif
        return true;
    }

    bool Model::set_component_lod_bias(const std::string& component, float bias) {
        bias = std::max(0.0f, bias);
        auto exec = [&](const std::string& name) -> bool {
            godot::MeshInstance3D* mesh = find_component(name);
            if (!mesh) return false;
            mesh->set_lod_bias(bias);
            return true;
        };
        if (!apply_wildcard(component, [&]{ return get_components(true); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("component '{}' not found in model '{}'", component, model_name));
        return true;
    }

    #if defined(VSDK_Client)
    bool Model::set_component_wireframe(const std::string& component, bool state) {
        const auto material = get_wireframe_material();
        auto exec = [&](const std::string& name) -> bool {
            godot::MeshInstance3D* mesh = find_component(name);
            if (!mesh) return false;
            if (state) mesh -> set_material_overlay(material);
            else if (mesh -> get_material_overlay().ptr() == material.ptr()) mesh -> set_material_overlay(godot::Ref<godot::Material>());
            return true;
        };
        if (!apply_wildcard(component, [&]{ return get_components(true); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("component '{}' not found in model '{}'", component, model_name));
        return true;
    }

    void Model::set_wireframe_all(bool state) {
        wireframe_all = state;
        godot::Window* root = Engine::Core::get_scene_root();
        if (!root) return;
        std::function<void(godot::Node*)> apply = [&](godot::Node* node) {
            if (auto model = godot::Object::cast_to<Model>(node)) {
                if (!model -> placeholder) model -> set_component_wireframe("*", state);
            }
            for (int i = 0; i < node -> get_child_count(); i++) apply(node -> get_child(i));
        };
        apply(root);
    }
    #endif

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
            else mesh->set_surface_override_material(idx, godot::Ref<godot::Material>());
            return true;
        };
        if (!apply_wildcard(material, [&]{ return get_materials(component); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("material '{}' not found in component '{}'", material, component));
        return true;
    }

    bool Model::set_material_feature(const std::string& component, const std::string& material, int feature, bool state) {
        assert_material_feature(feature);
        auto mesh = assert_component(component);
        auto exec = [&](const std::string& name) -> bool {
            return apply_standard_material(mesh, find_material_index(mesh, name), [&](godot::Ref<godot::StandardMaterial3D> mat) {
                mat->set_feature(static_cast<godot::BaseMaterial3D::Feature>(feature), state);
            });
        };
        if (!apply_wildcard(material, [&]{ return get_materials(component); }, exec)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("material '{}' not found in component '{}'", material, component));
        return true;
    }

    bool Model::set_material_flag(const std::string& component, const std::string& material, int flag, bool state) {
        assert_material_flag(flag);
        auto mesh = assert_component(component);
        auto exec = [&](const std::string& name) -> bool {
            return apply_standard_material(mesh, find_material_index(mesh, name), [&](godot::Ref<godot::StandardMaterial3D> mat) {
                mat->set_flag(static_cast<godot::BaseMaterial3D::Flags>(flag), state);
            });
        };
        if (!apply_wildcard(material, [&]{ return get_materials(component); }, exec)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("material '{}' not found in component '{}'", material, component));
        return true;
    }

    bool Model::set_blendshape_value(const std::string& component, const std::string& blend_shape, float value, bool sync) {
        auto mesh = assert_component(component);
        auto& recorded = blendshape_values[component];
        bool broadcast = false;
        auto exec = [&](const std::string& name) -> bool {
            int idx = mesh->find_blend_shape_by_name(Tool::to_godot_string(name));
            if (idx < 0) return false;
            mesh->set_blend_shape_value(idx, value);
            auto it = recorded.find(name);
            bool changed = it == recorded.end() || !godot::Math::is_equal_approx(it->second, value);
            recorded[name] = value;
            if (needs_broadcast(fmt::format("blendshape:{}:{}", component, name), changed, sync)) broadcast = true;
            return true;
        };
        if (!apply_wildcard(blend_shape, [&]{ return get_blendshapes(component); }, exec)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("blendshape '{}' not found in component '{}'", blend_shape, component));
        if (broadcast) broadcast_blendshape_value(component, blend_shape, value);
        return true;
    }

    void Model::set_animation_layer_speed(int layer, float speed, bool sync) {
        bool valid = layer >= 0 && layer < (int)anim_layers.size();
        bool changed = valid && !godot::Math::is_equal_approx(anim_layers[layer].speed, speed);
        apply_set_animation_layer_speed(layer, speed);
        if (valid && needs_broadcast(fmt::format("speed:{}", layer), changed, sync)) broadcast_animation_layer(3, layer, "", true, speed, 0.0f, 0.0f);
    }

    bool Model::set_animation_layer_weight(int layer, float weight, float blend_time, bool sync) {
        bool changed = layer > 0 && layer < (int)anim_layers.size() && !godot::Math::is_equal_approx(anim_layers[layer].weight_target, std::clamp(weight, 0.0f, 1.0f));
        if (!apply_set_animation_layer_weight(layer, weight, blend_time)) return false;
        if (needs_broadcast(fmt::format("weight:{}", layer), changed, sync)) broadcast_animation_layer(2, layer, "", true, 1.0f, weight, blend_time);
        return true;
    }

    bool Model::set_animation_layer_filter(int layer, bool enabled, const std::vector<std::string>& bone_paths, bool sync) {
        if (layer < 1) return false;
        if (!ensure_animation_layer(layer)) return false;
        const auto& current = anim_layers[layer];
        bool changed = current.filter_enabled != (enabled && !bone_paths.empty()) || current.filter_bones != (enabled ? bone_paths : std::vector<std::string>{});
        apply_set_animation_layer_filter(layer, enabled, bone_paths);
        if (needs_broadcast(fmt::format("filter:{}", layer), changed, sync)) broadcast_animation_layer_filter(layer, enabled, bone_paths);
        return true;
    }


    // Misc //
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

    void Model::collect_mesh_nodes(godot::Node* node, std::vector<std::string>& out, const std::string& current_path) {
        if (!node) return;
        for (int i = 0; i < node->get_child_count(); i++) {
            godot::Node* child = node->get_child(i);
            std::string child_name = Tool::to_std_string(child->get_name());
            bool is_generated = !child_name.empty() && child_name[0] == '@';
            std::string child_path = (current_path.empty() || is_generated) ? (is_generated ? "" : child_name) : fmt::format("{}/{}", current_path, child_name);
            if (!is_generated && godot::Object::cast_to<godot::MeshInstance3D>(child)) out.push_back(child_path);
            collect_mesh_nodes(child, out, child_path);
        }
    }

    bool Model::play_animation_layer(int layer, const std::string& name, bool loop, float speed, float weight, float blend_time, bool sync) {
        if (!apply_play_animation_layer(layer, name, loop, speed, weight, blend_time)) return false;
        if (resolve_sync(sync)) broadcast_animation_layer(0, layer, name, loop, speed, weight, blend_time);
        return true;
    }

    void Model::stop_animation_layer(int layer, float blend_time, bool sync) {
        apply_stop_animation_layer(layer, blend_time);
        if (resolve_sync(sync)) broadcast_animation_layer(1, layer, "", true, 1.0f, 0.0f, blend_time);
    }


    // Apply //
    bool Model::apply_component_visible(const std::string& component, bool state) {
        if (placeholder) {
            if (Tool::contains_wildcard(component)) {
                godot::UtilityFunctions::push_warning(
                    "Model::apply_component_visible — wildcard '", Tool::to_godot_string(component),
                    "' sync dropped: model '", Tool::to_godot_string(model_name), "' still loading");
                return false;
            }
            if (!state) component_hidden.insert(component);
            else        component_hidden.erase(component);
            return true;
        }

        auto exec = [&](const std::string& name) -> bool {
            godot::MeshInstance3D* mesh = find_mesh_node(this, name);
            if (!mesh) return false;
            mesh->set_visible(state);
            if (!state) component_hidden.insert(name);
            else        component_hidden.erase(name);
            return true;
        };
        if (!apply_wildcard(component, [&]{ return get_components(); }, exec)) {
            godot::UtilityFunctions::push_warning(
                "Model::apply_component_visible — component '", Tool::to_godot_string(component),
                "' not found in model '", Tool::to_godot_string(model_name), "'");
            return false;
        }
        return true;
    }

    bool Model::apply_component_rendered(const std::string& component, bool state) {
        if (placeholder) {
            if (Tool::contains_wildcard(component)) {
                godot::UtilityFunctions::push_warning(
                    "Model::apply_component_rendered — wildcard '", Tool::to_godot_string(component),
                    "' sync dropped: model '", Tool::to_godot_string(model_name), "' still loading");
                return false;
            }
            if (!state) component_detached.insert(component);
            else        component_detached.erase(component);
            return true;
        }

        if (state) {
            auto exec = [&](const std::string& name) -> bool {
                auto it = detached_components.find(name);
                if (it == detached_components.end()) return true;
                godot::MeshInstance3D* mesh = it->second;
                detached_components.erase(it);
                component_detached.erase(name);
                this->add_child(mesh);
                mesh->set_physics_process(true);
                mesh->set_process(true);
                return true;
            };
            if (Tool::contains_wildcard(component)) {
                std::vector<std::string> keys;
                keys.reserve(detached_components.size());
                for (const auto& [name, _] : detached_components) keys.push_back(name);
                for (const auto& name : keys)
                    if (Tool::match_wildcard(component, name)) exec(name);
                return true;
            }
            return exec(component);
        }
        else {
            auto exec = [&](const std::string& name) -> bool {
                if (detached_components.count(name)) return true;
                godot::MeshInstance3D* mesh = find_mesh_node(this, name);
                if (!mesh) return false;
                godot::Node* parent = mesh->get_parent();
                if (!parent) return false;
                mesh->set_physics_process(false);
                mesh->set_process(false);
                parent->remove_child(mesh);
                detached_components[name] = mesh;
                component_detached.insert(name);
                return true;
            };
            if (Tool::contains_wildcard(component)) {
                for (const auto& name : get_components())
                    if (Tool::match_wildcard(component, name)) exec(name);
                return true;
            }
            if (!exec(component)) {
                godot::UtilityFunctions::push_warning(
                    "Model::apply_component_rendered — component '", Tool::to_godot_string(component),
                    "' not found in model '", Tool::to_godot_string(model_name), "'");
                return false;
            }
            return true;
        }
    }

    bool Model::apply_component_transform(const std::string& component, const godot::Transform3D& transform) {
        if (!transform.origin.is_finite() || !transform.basis.get_column(0).is_finite() || !transform.basis.get_column(1).is_finite() || !transform.basis.get_column(2).is_finite()) return false;
        if (placeholder) {
            component_transforms[component] = transform;
            return true;
        }
        godot::MeshInstance3D* mesh = find_component(component);
        if (!mesh) {
            godot::UtilityFunctions::push_warning(
                "Model::apply_component_transform — component '", Tool::to_godot_string(component),
                "' not found in model '", Tool::to_godot_string(model_name), "'");
            return false;
        }
        mesh->set_transform(transform);
        component_transforms[component] = transform;
        return true;
    }

    bool Model::apply_blendshape_value(const std::string& component, const std::string& blend_shape, float value) {
        if (!godot::Math::is_finite(value)) return false;
        if (placeholder) {
            if (Tool::contains_wildcard(blend_shape)) {
                godot::UtilityFunctions::push_warning(
                    "Model::apply_blendshape_value — wildcard '", Tool::to_godot_string(blend_shape),
                    "' sync dropped: model '", Tool::to_godot_string(model_name), "' still loading");
                return false;
            }
            blendshape_values[component][blend_shape] = value;
            return true;
        }
        godot::MeshInstance3D* mesh = find_component(component);
        if (!mesh) {
            godot::UtilityFunctions::push_warning(
                "Model::apply_blendshape_value — component '", Tool::to_godot_string(component),
                "' not found in model '", Tool::to_godot_string(model_name), "'");
            return false;
        }
        auto& recorded = blendshape_values[component];
        auto exec = [&](const std::string& name) -> bool {
            int idx = mesh->find_blend_shape_by_name(Tool::to_godot_string(name));
            if (idx < 0) return false;
            mesh->set_blend_shape_value(idx, value);
            recorded[name] = value;
            return true;
        };
        if (!apply_wildcard(blend_shape, [&]{ return get_blendshapes(component); }, exec)) {
            godot::UtilityFunctions::push_warning(
                "Model::apply_blendshape_value — blendshape '", Tool::to_godot_string(blend_shape),
                "' not found in component '", Tool::to_godot_string(component), "'");
            return false;
        }
        return true;
    }

    int Model::apply_material_shader(const std::string& component, const std::string& material, godot::Ref<godot::ShaderMaterial> shader_material) {
        if (!shader_material.is_valid()) return apply_material_shader(component, material, SurfaceMaterialFactory{});
        return apply_material_shader(component, material, SurfaceMaterialFactory([shader_material](godot::Ref<godot::Material>) { return shader_material; }));
    }

    int Model::apply_material_shader(const std::string& component, const std::string& material, const SurfaceMaterialFactory& factory) {
        int count = 0;
        const bool wildcard_component = Tool::contains_wildcard(component);

        auto apply_to_component = [&](const std::string& comp_name) -> bool {
            godot::MeshInstance3D* mesh = find_mesh_node(this, comp_name);
            if (!mesh) return false;

            auto apply_to_surface = [&](const std::string& mat_name) -> bool {
                int idx = find_material_index(mesh, mat_name);
                if (idx < 0) return false;
                if (!factory) {
                    mesh -> set_surface_override_material(idx, godot::Ref<godot::Material>());
                }
                else {
                    godot::Ref<godot::Material> original;
                    if (mesh -> get_mesh().is_valid()) original = mesh -> get_mesh() -> surface_get_material(idx);
                    mesh -> set_surface_override_material(idx, factory(original));
                }
                count++;
                return true;
            };

            if (!apply_wildcard(material, [&]{ return get_materials(comp_name); }, apply_to_surface) && !wildcard_component)
                throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("material '{}' not found in component '{}'", material, comp_name));
            return true;
        };

        if (!apply_wildcard(component, [&]{ return get_components(); }, apply_to_component))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("component '{}' not found in model '{}'", component, model_name));
        if (wildcard_component && !Tool::contains_wildcard(material) && count == 0)
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("material '{}' not found in any component matching '{}' of model '{}'", material, component, model_name));
        return count;
    }


    // Broadcast //
    void Model::broadcast_component_visible(const std::string& component, bool state) {
        if (net_id == 0) return;
        Manager::Network::rpc(true, "_sync_component_visible", (int)net_id, Tool::to_godot_string(component), state);
    }

    void Model::broadcast_component_rendered(const std::string& component, bool state) {
        if (net_id == 0) return;
        Manager::Network::rpc(true, "_sync_component_render", (int)net_id, Tool::to_godot_string(component), state);
    }

    void Model::broadcast_component_transform(const std::string& component, const godot::Transform3D& transform) {
        if (net_id == 0) return;
        #if !defined(VSDK_Client)
        Manager::Network::rpc(true, "_sync_component_transform", (int)net_id, Tool::to_godot_string(component), transform);
        #else
        auto net_mgr = Manager::Network::get_singleton();
        if (!net_mgr || net_mgr->get_peer_id() != sync_authority) return;
        Manager::Network::rpc_id(false, 1, "_sync_component_transform", (int)net_id, Tool::to_godot_string(component), transform);
        #endif
    }

    void Model::broadcast_blendshape_value(const std::string& component, const std::string& blend_shape, float value) {
        if (net_id == 0) return;
        #if !defined(VSDK_Client)
        Manager::Network::rpc(true, "_sync_blendshape", (int)net_id, Tool::to_godot_string(component), Tool::to_godot_string(blend_shape), value);
        #else
        auto net_mgr = Manager::Network::get_singleton();
        if (!net_mgr || net_mgr->get_peer_id() != sync_authority) return;
        Manager::Network::rpc_id(false, 1, "_sync_blendshape", (int)net_id, Tool::to_godot_string(component), Tool::to_godot_string(blend_shape), value);
        #endif
    }


    // Sync //
    void Model::sync_component_transform(const std::string& component, bool sync) {
        auto transform = assert_component(component)->get_transform();
        auto it = component_transforms.find(component);
        bool changed = it == component_transforms.end() || !it->second.is_equal_approx(transform);
        component_transforms[component] = transform;
        if (needs_broadcast(fmt::format("transform:{}", component), changed, sync)) broadcast_component_transform(component, transform);
    }

    bool Model::needs_broadcast(const std::string& key, bool changed, bool sync) {
        if (!resolve_sync(sync)) {
            if (changed) unsynced_keys.insert(key);
            return false;
        }
        return unsynced_keys.erase(key) > 0 || changed;
    }
}