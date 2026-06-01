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
#include <Vital.sandbox/Engine/public/console.h>
#include <Vital.sandbox/Manager/public/asset.h>
#include <Vital.sandbox/Manager/public/resource.h>


///////////////////////////
// Vital: Engine: Model //
///////////////////////////

namespace Vital::Engine {
    // TODO: Improve

    //------------------------------//
    //  ModelSpawnerDelegate: Impl  //
    //------------------------------//

    godot::Object* ModelSpawnerDelegate::spawn(godot::Variant data) {
        godot::UtilityFunctions::print("ModelSpawnerDelegate::spawn called with -> ", data);
        std::string name = Tool::to_std_string(data.operator godot::String());

        auto it = Model::cache_loaded.find(name);
        if (it != Model::cache_loaded.end()) {
            Model* object = memnew(Model);
            object -> set_model_name(name);
            godot::Node* instance = it -> second -> instantiate();
            if (!instance) {
                memdelete(object);
                return nullptr;
            }
            object -> remote = true;
            object -> add_child(instance);
            godot::UtilityFunctions::print("ModelSpawnerDelegate::spawn — created: ", data);
            if (Model::on_spawned_callback) Model::on_spawned_callback(object);
            return object;
        }

        #if defined(Vital_SDK_Client)
            godot::UtilityFunctions::print("ModelSpawnerDelegate::spawn — asset not ready, creating placeholder: ", data);
            Model* placeholder = memnew(Model);
            placeholder -> set_model_name(name);
            placeholder -> placeholder = true;
            placeholder -> set_visible(false);
            placeholder -> setup_sync(1);
            Manager::Asset::get_singleton() -> queue_spawn(name, placeholder);
            return placeholder;
        #else
            godot::UtilityFunctions::print("ModelSpawnerDelegate::spawn — not in cache (server): ", data);
            return nullptr;
        #endif
    }


    // Instantiators //
    void Model::_ready() {
        godot::UtilityFunctions::print("Model::_ready fired, pending_authority=", pending_authority);
        if (placeholder) return;
        find_node(this, skeleton);
        find_node(this, anim_player);
        setup_sync(pending_authority);
    }

    void Model::_notification(int what) {
        if (what == NOTIFICATION_PREDELETE) {
            if (on_destroyed_callback) on_destroyed_callback(this);
        }
    }


    // Helpers //
    godot::MeshInstance3D* Model::find_mesh_node(godot::Node* node, const std::string& path) {
        if (!node) return nullptr;
        auto separator = path.find('/');
        std::string segment = (separator == std::string::npos) ? path : path.substr(0, separator);
        std::string remainder = (separator == std::string::npos) ? "" : path.substr(separator + 1);
        for (int i = 0; i < node -> get_child_count(); i++) {
            godot::Node* child = node -> get_child(i);
            std::string child_name = Tool::to_std_string(child -> get_name());
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
        godot::ArrayMesh* array_mesh = godot::Object::cast_to<godot::ArrayMesh>(mesh -> get_mesh().ptr());
        if (!array_mesh) return -1;
        for (int i = 0; i < array_mesh -> get_surface_count(); i++) {
            if (Tool::to_std_string(array_mesh -> surface_get_name(i)) == material) return i;
        }
        return -1;
    }

    void Model::collect_mesh_nodes(godot::Node* node, std::vector<std::string>& out, const std::string& current_path) {
        if (!node) return;
        for (int i = 0; i < node -> get_child_count(); i++) {
            godot::Node* child = node -> get_child(i);
            std::string child_name = Tool::to_std_string(child -> get_name());
            bool is_generated = !child_name.empty() && child_name[0] == '@';
            std::string child_path = (current_path.empty() || is_generated) ? (is_generated ? "" : child_name) : current_path + "/" + child_name;
            if (!is_generated && godot::Object::cast_to<godot::MeshInstance3D>(child)) out.push_back(child_path);
            collect_mesh_nodes(child, out, child_path);
        }
    }

    void Model::setup_sync(int authority_peer) {
        if (net_sync) return;

        auto config = memnew(godot::SceneReplicationConfig);
        config -> add_property(godot::NodePath(".:position"));
        config -> property_set_replication_mode(godot::NodePath(".:position"), godot::SceneReplicationConfig::REPLICATION_MODE_ALWAYS);
        config -> add_property(godot::NodePath(".:rotation"));
        config -> property_set_replication_mode(godot::NodePath(".:rotation"), godot::SceneReplicationConfig::REPLICATION_MODE_ALWAYS);

        net_sync = memnew(godot::MultiplayerSynchronizer);
        net_sync -> set_name("NetSync");
        net_sync -> set_replication_config(config);
        net_sync -> set_root_path(godot::NodePath(".."));
        net_sync -> set_multiplayer_authority(authority_peer);
        add_child(net_sync);
    }


    // Asserts //
    godot::MeshInstance3D* Model::assert_component(const std::string& component) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: component '{}' not found in model '{}'", component, model_name));
        return mesh;
    }

    std::pair<godot::MeshInstance3D*, int> Model::assert_material(const std::string& component, const std::string& material) {
        auto mesh = assert_component(component);
        int index = find_material_index(mesh, material);
        if (index < 0) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: material '{}' not found in component '{}'", material, component));
        return { mesh, index };
    }

    godot::Skeleton3D* Model::assert_skeleton() {
        if (!skeleton) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: no skeleton found in model '{}'", model_name));
        return skeleton;
    }

    godot::AnimationPlayer* Model::assert_animation_player() {
        if (!anim_player) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: no animation player found in model '{}'", model_name));
        return anim_player;
    }

    void Model::assert_material_feature(int feature) {
        if (feature < 0 || feature >= godot::BaseMaterial3D::FEATURE_MAX) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid material feature");
    }

    void Model::assert_material_flag(int flag) {
        if (flag < 0 || flag >= godot::BaseMaterial3D::FLAG_MAX) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid material flag");
    }


    //---------------------------//
    //  Spawner Setup (private)  //
    //---------------------------//

    void Model::setup_spawner() {
        if (net_spawner && net_spawner -> is_inside_tree()) return;
        if (net_spawner) teardown_spawner();
        auto core = Core::get_singleton();
        if (!core) return;

        net_spawner_delegate = memnew(ModelSpawnerDelegate);
        net_spawner_delegate -> set_name("ModelSpawnerDelegate");
        core -> add_child(net_spawner_delegate);

        net_spawner = memnew(godot::MultiplayerSpawner);
        net_spawner -> set_name("ModelSpawner");
        net_spawner -> set_spawn_function(godot::Callable(net_spawner_delegate, "spawn"));
        core -> add_child(net_spawner);
        net_spawner -> set_multiplayer_authority(1);
        net_spawner -> set_spawn_path(godot::NodePath(".."));

        godot::UtilityFunctions::print("ModelSpawner: watching path -> ", net_spawner -> get_spawn_path());
        godot::UtilityFunctions::print("ModelSpawner: authority -> ", net_spawner -> get_multiplayer_authority());
    }

    void Model::teardown_spawner() {
        if (net_spawner_delegate) {
            net_spawner_delegate -> queue_free();
            net_spawner_delegate = nullptr;
        }
        if (net_spawner) {
            net_spawner -> queue_free();
            net_spawner = nullptr;
        }
    }

    void Model::cleanup_spawned() {
        auto core = Engine::Core::get_singleton();
        if (!core) return;
        for (int i = core -> get_child_count() - 1; i >= 0; i--) {
            godot::Node* child = core -> get_child(i);
            if (godot::Object::cast_to<Model>(child)) child -> queue_free();
        }
    }

    void Model::on_connected() {
        Manager::Asset::get_singleton() -> clear();
        cleanup_spawned();
        if (net_spawner) {
            net_spawner -> set_multiplayer_authority(1);
            godot::UtilityFunctions::print("ModelSpawner: refreshed for new session");
        }
    }


    // Managers //
    bool Model::load(const std::string& name, const std::string& path) {
        return load_from_buffer(name, Tool::File::read_binary(Tool::get_directory(), path));
    }

    bool Model::load_from_buffer(const std::string& name, const godot::PackedByteArray& buffer) {
        if (is_model_loaded(name)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: model '{}' is already loaded", name));
    
        godot::Ref<godot::PackedScene> scene;
        switch (Tool::Format::get_format(format_registry, Format::UNKNOWN, buffer)) {
            case Format::GLB: {
                godot::Ref<godot::GLTFDocument> document = memnew(godot::GLTFDocument);
                godot::Ref<godot::GLTFState> state = memnew(godot::GLTFState);
                if (document -> append_from_buffer(buffer, "", state) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid model buffer");
                godot::Node* root = document -> generate_scene(state);
                if (!root) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: failed to generate scene");
                scene = godot::Ref<godot::PackedScene>(memnew(godot::PackedScene));
                scene -> pack(root);
                memdelete(root);
                break;
            }
        }
    
        if (scene.is_null()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: unsupported or invalid model format");
        cache_loaded[name] = scene;
        #if defined(Vital_SDK_Client)
        Manager::Asset::get_singleton() -> flush_spawn_queue(name);
        #endif
        return true;
    }

    bool Model::unload(const std::string& name) {
        auto it = cache_loaded.find(name);
        if (it == cache_loaded.end()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: model '{}' isn't loaded yet", name));
        cache_loaded.erase(it);
        return true;
    }

    Model* Model::create(const std::string& name, int authority_peer) {
        auto it = cache_loaded.find(name);
        if (it == cache_loaded.end()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: model '{}' isn't loaded yet", name));

        #if defined(Vital_SDK_Client)
            Model* object = memnew(Model);
            object -> set_model_name(name);
            godot::Node* instance = it -> second -> instantiate();
            if (!instance) {
                memdelete(object);
                throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: failed to instantiate model '{}'", name));
            }
            object -> add_child(instance);
            Engine::Core::get_singleton() -> add_child(object);
            return object;
        #else
            if (!net_spawner) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: spawner not ready — cannot create synced model '{}'", name));
            godot::Node* spawned = net_spawner -> spawn(Tool::to_godot_string(name));
            if (!spawned) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: spawner returned null for model '{}'", name));
            Model* object = godot::Object::cast_to<Model>(spawned);
            if (!object) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: spawned node is not a Model for '{}'", name));
            object -> pending_authority = authority_peer;
            godot::UtilityFunctions::print("ModelSpawner: spawned -> ", Tool::to_godot_string(name));
            return object;
        #endif
    }

    void Model::destroy() {
        this -> queue_free();
    }

    #if defined(Vital_SDK_Client)
    void Model::hydrate(int authority_peer) {
        if (!placeholder) return;

        auto it = cache_loaded.find(model_name);
        if (it == cache_loaded.end()) {
            godot::UtilityFunctions::push_warning("Model::hydrate — model not in cache: ", Tool::to_godot_string(model_name));
            return;
        }

        godot::Node* instance = it -> second -> instantiate();
        if (!instance) {
            godot::UtilityFunctions::push_warning("Model::hydrate — failed to instantiate: ", Tool::to_godot_string(model_name));
            return;
        }

        placeholder = false;
        pending_authority = authority_peer;
        remote = true;
        add_child(instance);

        // net_sync already exists from spawn() — no need to call setup_sync again.
        find_node(this, skeleton);
        find_node(this, anim_player);
        set_visible(true);
        if (on_spawned_callback) on_spawned_callback(this);
        godot::UtilityFunctions::print("Model::hydrate — placeholder hydrated: ", Tool::to_godot_string(model_name));
    }
    #endif

    std::vector<std::string> Model::filter_resource_models(const std::string& resource, const std::vector<std::string>& files) {
        std::vector<std::string> validated;
        for (const auto& file : files) {
            if (!Tool::Format::is_supported_extension(format_registry, file)) continue;
            if (!Tool::Format::is_supported_format(format_registry, Format::UNKNOWN, fmt::format("resources/{}/{}", resource, file))) continue;
            validated.push_back(file);
        }
        return validated;
    }
    
    void Model::load_resource_models(const std::string& resource, const std::vector<std::string>& files) {
        auto rm = Vital::Manager::Resource::get_singleton();
        std::vector<std::string> loaded;
        std::vector<std::string> failed;
        for (const auto& file : files) {
            if (!Tool::Format::is_supported_extension(format_registry, file)) continue;
            const std::string model_name = fmt::format(":{}/{}", resource, file);
            if (is_model_loaded(model_name)) continue;
            const std::string local_path = fmt::format("resources/{}/{}", resource, file);
            if (!Tool::Format::is_supported_format(format_registry, Format::UNKNOWN, local_path)) continue;
            try {
                load(model_name, local_path);
                loaded.push_back(model_name);
            }
            catch (...) { failed.push_back(file); }
        }
        if (!loaded.empty()) {
            std::string report = fmt::format("resource `{}` registered {} model asset(s):\n", resource, loaded.size());
            for (const auto& name : loaded) report += fmt::format("> `{}`\n", name);
            rm -> log("sbox", report);
        }
        if (!failed.empty()) {
            std::string report = fmt::format("resource `{}` failed to load {} model asset(s):\n", resource, failed.size());
            for (const auto& name : failed) report += fmt::format("> `{}`\n", name);
            rm -> log("error", report);
        }
    }
    
    void Model::unload_resource_models(const std::string& resource) {
        auto rm = Vital::Manager::Resource::get_singleton();
        const std::string prefix = fmt::format(":{}/", resource);
        std::vector<std::string> to_unload;
        for (const auto& [name, _] : cache_loaded) {
            if (name.rfind(prefix, 0) == 0) to_unload.push_back(name);
        }
        for (const auto& name : to_unload) {
            try { unload(name); }
            catch (...) {}
        }
        if (!to_unload.empty()) {
            rm -> log("sbox", fmt::format("resource `{}` unloaded {} model asset(s)", resource, to_unload.size()));
        }
    }


    // Checkers //
    bool Model::is_model_loaded(const std::string& name) {
        return cache_loaded.find(name) != cache_loaded.end();
    }

    bool Model::is_remote() const {
        return remote;
    }    

    bool Model::is_streamed() const {
        if (placeholder || !is_visible_in_tree()) return false;
        #if defined(Vital_SDK_Client)
            auto viewport = get_viewport();
            if (!viewport) return false;
            auto camera = viewport -> get_camera_3d();
            if (!camera) return false;
            return camera -> is_position_in_frustum(get_global_position());
        #else
            return true;
        #endif
    }
    
    bool Model::is_component_visible(const std::string& component) {
        return assert_component(component) -> is_visible();
    }

    bool Model::is_material_visible(const std::string& component, const std::string& material) {
        auto [mesh, index] = assert_material(component, material);
        return !mesh -> get_surface_override_material(index).is_valid();
    }

    bool Model::is_material_feature(const std::string& component, const std::string& material, int feature) {
        assert_material_feature(feature);
        auto [mesh, index] = assert_material(component, material);
        godot::Ref<godot::StandardMaterial3D> std_mat = godot::Object::cast_to<godot::StandardMaterial3D>(mesh -> get_active_material(index).ptr());
        if (!std_mat.is_valid()) return false;
        return std_mat -> get_feature(static_cast<godot::BaseMaterial3D::Feature>(feature));
    }

    bool Model::is_material_flag(const std::string& component, const std::string& material, int flag) {
        assert_material_flag(flag);
        auto [mesh, index] = assert_material(component, material);
        godot::Ref<godot::StandardMaterial3D> std_mat = godot::Object::cast_to<godot::StandardMaterial3D>(mesh -> get_active_material(index).ptr());
        if (!std_mat.is_valid()) return false;
        return std_mat -> get_flag(static_cast<godot::BaseMaterial3D::Flags>(flag));
    }

    bool Model::is_animation_playing() {
        return anim_player && anim_player -> is_playing();
    }


    // Getters //
    Model::Models Model::get_loaded_models() {
        return cache_loaded;
    }

    std::string Model::get_model_name() {
        return model_name;
    }

    godot::Vector3 Model::get_position() {
        return get_global_position();
    }

    godot::Vector3 Model::get_rotation() {
        return get_rotation_degrees();
    }

    std::vector<std::string> Model::get_components() {
        std::vector<std::string> components;
        collect_mesh_nodes(this, components, "");
        return components;
    }

    std::vector<std::string> Model::get_materials(const std::string& component) {
        godot::ArrayMesh* array_mesh = godot::Object::cast_to<godot::ArrayMesh>(assert_component(component) -> get_mesh().ptr());
        std::vector<std::string> materials;
        if (!array_mesh) return materials;
        for (int i = 0; i < array_mesh -> get_surface_count(); i++) {
            materials.push_back(Tool::to_std_string(array_mesh -> surface_get_name(i)));
        }
        return materials;
    }

    std::vector<std::string> Model::get_blendshapes(const std::string& component) {
        auto mesh = assert_component(component);
        godot::ArrayMesh* array_mesh = godot::Object::cast_to<godot::ArrayMesh>(mesh -> get_mesh().ptr());
        std::vector<std::string> blendshapes;
        if (!array_mesh) return blendshapes;
        for (int i = 0; i < mesh -> get_blend_shape_count(); i++) {
            blendshapes.push_back(Tool::to_std_string(array_mesh -> get_blend_shape_name(i)));
        }
        return blendshapes;
    }

    std::vector<std::string> Model::get_bones() {
        std::vector<std::string> bones;
        if (skeleton) {
            for (int i = 0; i < skeleton -> get_bone_count(); i++) {
                bones.push_back(Tool::to_std_string(skeleton -> get_bone_name(i)));
            }
        }
        return bones;
    }

    std::vector<std::string> Model::get_animations() {
        std::vector<std::string> animations;
        if (anim_player) {
            auto animation_list = anim_player -> get_animation_list();
            for (int i = 0; i < animation_list.size(); i++) {
                animations.push_back(Tool::to_std_string(animation_list[i]));
            }
        }
        return animations;
    }

    float Model::get_blendshape_value(const std::string& component, const std::string& blend_shape) {
        auto mesh = assert_component(component);
        int index = mesh -> find_blend_shape_by_name(Tool::to_godot_string(blend_shape));
        if (index < 0) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: blendshape '{}' not found in component '{}'", blend_shape, component));
        return mesh -> get_blend_shape_value(index);
    }

    godot::Vector3 Model::get_bone_position(const std::string& bone) {
        auto skeleton = assert_skeleton();
        int index = skeleton -> find_bone(Tool::to_godot_string(bone));
        if (index == -1) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: bone '{}' not found in model '{}'", bone, model_name));
        return skeleton -> get_global_transform().xform(skeleton -> get_bone_global_pose(index).origin);
    }

    std::string Model::get_current_animation() {
        return Tool::to_std_string(assert_animation_player() -> get_current_animation());
    }

    float Model::get_animation_speed() {
        return assert_animation_player() -> get_speed_scale();
    }

    int Model::get_sync_authority() const {
        if (!net_sync) return 0;
        return net_sync -> get_multiplayer_authority();
    }


    // Setters //
    void Model::set_model_name(const std::string& name) {
        model_name = name;
    }

    void Model::set_position(godot::Vector3 position) {
        Engine::Core::get_singleton() -> enqueue([this, position]() {
            set_global_position(position);
        });
    }

    void Model::set_rotation(godot::Vector3 rotation) {
        Engine::Core::get_singleton() -> enqueue([this, rotation]() {
            set_rotation_degrees(rotation);
        });
    }

    #if !defined(Vital_SDK_Client)
    void Model::set_sync_authority(int peer_id) {
        if (!net_sync) return;
        net_sync -> set_multiplayer_authority(peer_id);
    }
    #endif

    bool Model::set_component_visible(const std::string& component, bool state) {
        auto exec = [&](const std::string& name) -> bool {
            godot::MeshInstance3D* mesh = find_mesh_node(this, name);
            if (!mesh) return false;
            mesh -> set_visible(state);
            return true;
        };
        if (!apply_wildcard(component, [&]{ return get_components(); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: component '{}' not found in model '{}'", component, model_name));
        return true;
    }

    bool Model::set_material_visible(const std::string& component, const std::string& material, bool state) {
        auto mesh = assert_component(component);
        auto exec = [&](const std::string& name) -> bool {
            int index = find_material_index(mesh, name);
            if (index < 0) return false;
            if (!state) {
                godot::Ref<godot::StandardMaterial3D> invisible = godot::Ref<godot::StandardMaterial3D>(memnew(godot::StandardMaterial3D));
                invisible -> set_transparency(godot::BaseMaterial3D::TRANSPARENCY_ALPHA);
                invisible -> set_depth_draw_mode(godot::BaseMaterial3D::DEPTH_DRAW_DISABLED);
                invisible -> set_albedo(godot::Color(0, 0, 0, 0));
                mesh -> set_surface_override_material(index, invisible);
            }
            else mesh -> set_surface_override_material(index, godot::Ref<godot::Material>());
            return true;
        };
        if (!apply_wildcard(material, [&]{ return get_materials(component); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: material '{}' not found in component '{}'", material, component));
        return true;
    }

    bool Model::set_material_feature(const std::string& component, const std::string& material, int feature, bool state) {
        assert_material_feature(feature);
        auto mesh = assert_component(component);
        auto exec = [&](const std::string& name) -> bool {
            return apply_standard_material(mesh, find_material_index(mesh, name), [&](godot::Ref<godot::StandardMaterial3D> mat) {
                mat -> set_feature(static_cast<godot::BaseMaterial3D::Feature>(feature), state);
            });
        };
        if (!apply_wildcard(material, [&]{ return get_materials(component); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: material '{}' not found in component '{}'", material, component));
        return true;
    }

    bool Model::set_material_flag(const std::string& component, const std::string& material, int flag, bool state) {
        assert_material_flag(flag);
        auto mesh = assert_component(component);
        auto exec = [&](const std::string& name) -> bool {
            return apply_standard_material(mesh, find_material_index(mesh, name), [&](godot::Ref<godot::StandardMaterial3D> mat) {
                mat -> set_flag(static_cast<godot::BaseMaterial3D::Flags>(flag), state);
            });
        };
        if (!apply_wildcard(material, [&]{ return get_materials(component); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: material '{}' not found in component '{}'", material, component));
        return true;
    }

    bool Model::set_blendshape_value(const std::string& component, const std::string& blend_shape, float value) {
        auto mesh = assert_component(component);
        auto exec = [&](const std::string& name) -> bool {
            int index = mesh -> find_blend_shape_by_name(Tool::to_godot_string(name));
            if (index < 0) return false;
            mesh -> set_blend_shape_value(index, value);
            return true;
        };
        if (!apply_wildcard(blend_shape, [&]{ return get_blendshapes(component); }, exec))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: blendshape '{}' not found in component '{}'", blend_shape, component));
        return true;
    }

    void Model::set_animation_speed(float speed) {
        assert_animation_player() -> set_speed_scale(speed);
    }


    // APIs //
    bool Model::play_animation(const std::string& name, bool loop, float speed) {
        if (!anim_player) return false;
        if (!anim_player -> has_animation(Tool::to_godot_string(name))) {
            godot::UtilityFunctions::push_warning("Animation '", Tool::to_godot_string(name), "' not found in model '", Tool::to_godot_string(model_name), "'");
            return false;
        }
        godot::Ref<godot::Animation> animation = anim_player -> get_animation(Tool::to_godot_string(name));
        if (animation.is_valid()) animation -> set_loop_mode(loop ? godot::Animation::LOOP_LINEAR : godot::Animation::LOOP_NONE);
        anim_player -> set_speed_scale(speed);
        anim_player -> play(Tool::to_godot_string(name));
        return true;
    }

    void Model::stop_animation() {
        if (!anim_player) return;
        anim_player -> stop();
    }

    void Model::pause_animation() {
        if (!anim_player) return;
        anim_player -> pause();
    }

    void Model::resume_animation() {
        if (!anim_player) return;
        auto current = anim_player -> get_current_animation();
        if (current.is_empty()) return;
        anim_player -> play(current);
    }
}