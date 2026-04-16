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
        if (it == Model::cache_loaded.end()) {
            godot::UtilityFunctions::print("ModelSpawnerDelegate::spawn — not in cache: ", data);
            return nullptr;
        }
        Model* object = memnew(Model);
        object -> set_model_name(name);
        godot::Node* instance = it->second->instantiate();
        if (!instance) {
            memdelete(object);
            return nullptr;
        }
        object -> add_child(instance);
        godot::UtilityFunctions::print("ModelSpawnerDelegate::spawn — created: ", data);
        return object;
    }


    // Instantiators //
    void Model::_ready() {
        godot::UtilityFunctions::print("Model::_ready fired, pending_authority=", pending_authority);
        find_skeleton(this);
        find_animation_player(this);
        setup_sync(pending_authority);
    }


    // Helpers //
    godot::MeshInstance3D* Model::find_mesh_node(godot::Node* node, const std::string& path) {
        if (!node) return nullptr;
        auto separator = path.find('/');
        std::string segment = (separator == std::string::npos) ? path : path.substr(0, separator);
        std::string remainder = (separator == std::string::npos) ? "" : path.substr(separator + 1);
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
        godot::ArrayMesh* array_mesh = godot::Object::cast_to<godot::ArrayMesh>(mesh -> get_mesh().ptr());
        if (!array_mesh) return -1;
        for (int i = 0; i < array_mesh->get_surface_count(); i++) {
            if (Tool::to_std_string(array_mesh->surface_get_name(i)) == material) return i;
        }
        return -1;
    }

    godot::Skeleton3D* Model::find_skeleton(godot::Node* node) {
        if (!node || skeleton) return skeleton;
        auto result = godot::Object::cast_to<godot::Skeleton3D>(node);
        if (result) { skeleton = result; return skeleton; }
        for (int i = 0; i < node->get_child_count(); i++) {
            auto result = find_skeleton(node->get_child(i));
            if (result) break;
        }
        return skeleton;
    }

    godot::AnimationPlayer* Model::find_animation_player(godot::Node* node) {
        if (!node || animation_player) return animation_player;
        auto result = godot::Object::cast_to<godot::AnimationPlayer>(node);
        if (result) { animation_player = result; return animation_player; }
        for (int i = 0; i < node->get_child_count(); i++) {
            auto result = find_animation_player(node->get_child(i));
            if (result) break;
        }
        return animation_player;
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

    void Model::setup_sync(int authority_peer) {
        if (net_sync) return;

        auto config = memnew(godot::SceneReplicationConfig);
        config->add_property(godot::NodePath(".:position"));
        config->property_set_replication_mode(godot::NodePath(".:position"), godot::SceneReplicationConfig::REPLICATION_MODE_ALWAYS);
        config->add_property(godot::NodePath(".:rotation"));
        config->property_set_replication_mode(godot::NodePath(".:rotation"), godot::SceneReplicationConfig::REPLICATION_MODE_ALWAYS);

        net_sync = memnew(godot::MultiplayerSynchronizer);
        net_sync->set_name("NetSync");
        net_sync->set_replication_config(config);
        net_sync->set_root_path(godot::NodePath(".."));
        net_sync->set_multiplayer_authority(authority_peer);
        add_child(net_sync);
    }


    // Asserts //
    godot::MeshInstance3D* Model::assert_component(const std::string& component) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: component '{}' not found in model '{}'", component, model_name));
        return mesh;
    }

    std::pair<godot::MeshInstance3D*, int> Model::assert_material(const std::string& component, const std::string& material) {
        godot::MeshInstance3D* mesh = assert_component(component);
        int index = find_material_index(mesh, material);
        if (index < 0) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: material '{}' not found in component '{}'", material, component));
        return { mesh, index };
    }

    void Model::assert_material_feature(int feature) {
        if (feature < 0 || feature >= godot::BaseMaterial3D::FEATURE_MAX) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "\n> Reason: invalid material feature");
    }

    void Model::assert_material_flag(int flag) {
        if (flag < 0 || flag >= godot::BaseMaterial3D::FLAG_MAX) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "\n> Reason: invalid material flag");
    }


    //---------------------------//
    //  Spawner Setup (private)  //
    //---------------------------//

    void Model::setup_spawner() {
        if (net_spawner && net_spawner->is_inside_tree()) return;
        if (net_spawner) teardown_spawner();
        auto core = Core::get_singleton();
        if (!core) return;

        net_spawner_delegate = memnew(ModelSpawnerDelegate);
        net_spawner_delegate->set_name("ModelSpawnerDelegate");
        core->add_child(net_spawner_delegate);

        net_spawner = memnew(godot::MultiplayerSpawner);
        net_spawner->set_name("ModelSpawner");
        net_spawner->set_spawn_function(godot::Callable(net_spawner_delegate, "spawn"));
        core->add_child(net_spawner);
        net_spawner->set_multiplayer_authority(1);
        net_spawner->set_spawn_path(godot::NodePath(".."));

        godot::UtilityFunctions::print("ModelSpawner: watching path -> ", net_spawner->get_spawn_path());
        godot::UtilityFunctions::print("ModelSpawner: authority -> ", net_spawner->get_multiplayer_authority());
    }

    void Model::teardown_spawner() {
        cache_synced.clear();
        if (net_spawner_delegate) {
            net_spawner_delegate->queue_free();
            net_spawner_delegate = nullptr;
        }
        if (net_spawner) {
            net_spawner->queue_free();
            net_spawner = nullptr;
        }
    }

    void Model::cleanup_spawned() {
        auto core = Core::get_singleton();
        if (!core) return;
        for (int i = core->get_child_count() - 1; i >= 0; i--) {
            godot::Node* child = core->get_child(i);
            if (godot::Object::cast_to<Model>(child)) child->queue_free();
        }
        cache_synced.clear();
    }

    void Model::clear_synced() {
        cache_synced.clear();
    }

    void Model::on_connected() {
        Manager::Asset::get_singleton() -> clear();
        cleanup_spawned();
        if (net_spawner) {
            net_spawner->set_multiplayer_authority(1);
            godot::UtilityFunctions::print("ModelSpawner: refreshed for new session");
        }
    }


    // Managers //
    bool Model::load(const std::string& name, const std::string& path) {
        return load_from_buffer(name, Tool::File::read_binary(Tool::get_directory(), path));
    }

    bool Model::load_from_buffer(const std::string& name, const godot::PackedByteArray& buffer) {
        if (is_model_loaded(name)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: model '{}' is already loaded", name));
        godot::Ref<godot::PackedScene> scene;
        switch (get_format(buffer)) {
            case Format::GLB: {
                godot::Ref<godot::GLTFDocument> document = memnew(godot::GLTFDocument);
                godot::Ref<godot::GLTFState> state = memnew(godot::GLTFState);
                if (document->append_from_buffer(buffer, "", state) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "\n> Reason: invalid model buffer");
                godot::Node* root = document->generate_scene(state);
                if (!root) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "\n> Reason: failed to generate scene");
                scene = godot::Ref<godot::PackedScene>(memnew(godot::PackedScene));
                scene->pack(root);
                memdelete(root);
                break;
            }
        }
        if (scene.is_null()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "\n> Reason: no scene detected");
        cache_loaded[name] = scene;
        return true;
    }

    bool Model::unload(const std::string& name) {
        auto it = cache_loaded.find(name);
        if (it == cache_loaded.end()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: model '{}' isn't loaded yet", name));
        cache_loaded.erase(it);
        cache_synced.erase(name);
        return true;
    }

    Model* Model::create(const std::string& name) {
        auto it = cache_loaded.find(name);
        if (it == cache_loaded.end()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: model '{}' isn't loaded yet", name));
        Model* object = memnew(Model);
        object -> set_model_name(name);
        godot::Node* instance = it->second->instantiate();
        if (!instance) {
            memdelete(object);
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, fmt::format("\n> Reason: failed to instantiate model '{}'", name));
        }
        object -> add_child(instance);
        Core::get_singleton() -> add_child(object);
        return object;
    }

    void Model::create_synced(const std::string& name, int authority_peer) {
        if (cache_loaded.find(name) == cache_loaded.end()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: model '{}' isn't loaded yet", name));
        Core::get_singleton() -> push_deferred([name, authority_peer]() {
            spawn_synced(name, authority_peer);
        });
    }

    Model* Model::spawn_synced(const std::string& name, int authority_peer) {
        if (!net_spawner) {
            godot::UtilityFunctions::print("ModelSpawner: spawn_synced — spawner not ready");
            return nullptr;
        }
        godot::Node* spawned = net_spawner->spawn(Tool::to_godot_string(name));
        if (!spawned) {
            godot::UtilityFunctions::print("ModelSpawner: spawn() returned null");
            return nullptr;
        }
        Model* object = godot::Object::cast_to<Model>(spawned);
        if (!object) {
            godot::UtilityFunctions::print("ModelSpawner: spawned node is not a Model");
            return nullptr;
        }
        object -> pending_authority = authority_peer;
        cache_synced[name] = object;
        godot::UtilityFunctions::print("ModelSpawner: spawned -> ", Tool::to_godot_string(name));
        return object;
    }

    Model* Model::get_synced(const std::string& name) {
        auto it = cache_synced.find(name);
        return it != cache_synced.end() ? it->second : nullptr;
    }

    void Model::destroy() {
        for (auto it = cache_synced.begin(); it != cache_synced.end(); ++it) {
            if (it->second == this) {
                cache_synced.erase(it);
                break;
            }
        }
        this->queue_free();
    }


    // Checkers //
    bool Model::is_model_loaded(const std::string& name) {
        return cache_loaded.find(name) != cache_loaded.end();
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
        return animation_player && animation_player->is_playing();
    }

    bool Model::is_synced() const {
        return net_sync != nullptr;
    }


    // Setters //
    void Model::set_model_name(const std::string& name) {
        model_name = name;
    }

    void Model::set_position(godot::Vector3 position) {
        Core::get_singleton() -> push_deferred([this, position]() {
            set_global_position(position);
        });
    }

    void Model::set_rotation(godot::Vector3 rotation) {
        Core::get_singleton() -> push_deferred([this, rotation]() {
            set_rotation_degrees(rotation);
        });
    }

    bool Model::set_component_visible(const std::string& component, bool state) {
        auto exec = [&](const std::string& name) {
            godot::MeshInstance3D* mesh = find_mesh_node(this, name);
            if (!mesh) return false;
            mesh -> set_visible(state);
            return true;
        };
        if (Tool::contains_wildcard(component)) {
            for (const auto& name : get_components()) {
                if (Tool::match_wildcard(component, name)) exec(name);
            }
        }
        else if (!exec(component)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: component '{}' not found in model '{}'", component, model_name));
        return true;
    }

    bool Model::set_material_visible(const std::string& component, const std::string& material, bool state) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: component '{}' not found in model '{}'", component, model_name));
        auto exec = [&](int index) {
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
        if (Tool::contains_wildcard(material)) {
            for (const auto& name : get_materials(component)) {
                if (Tool::match_wildcard(material, name)) exec(find_material_index(mesh, name));
            }
        }
        else if (!exec(find_material_index(mesh, material))) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: material '{}' not found in component '{}'", material, component));
        return true;
    }

    bool Model::set_material_feature(const std::string& component, const std::string& material, int feature, bool state) {
        validate_material_feature(feature);
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: component '{}' not found in model '{}'", component, model_name));
        auto exec = [&](int index) {
            if (index < 0) return false;
            godot::Ref<godot::Material> mat = mesh->get_active_material(index);
            godot::Ref<godot::StandardMaterial3D> std_mat = godot::Object::cast_to<godot::StandardMaterial3D>(mat.ptr());
            if (!std_mat.is_valid()) {
                if (mat.is_valid()) return false;
                std_mat = godot::Ref<godot::StandardMaterial3D>(memnew(godot::StandardMaterial3D));
                mesh->set_surface_override_material(index, std_mat);
            }
            std_mat->set_feature(static_cast<godot::BaseMaterial3D::Feature>(feature), state);
            return true;
        };
        if (Tool::contains_wildcard(material)) {
            for (const auto& name : get_materials(component)) {
                if (Tool::match_wildcard(material, name)) exec(find_material_index(mesh, name));
            }
        }
        else if (!exec(find_material_index(mesh, material))) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: material '{}' not found in component '{}'", material, component));
        return true;
    }

    bool Model::set_material_flag(const std::string& component, const std::string& material, int flag, bool state) {
        validate_material_flag(flag);
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: component '{}' not found in model '{}'", component, model_name));
        auto exec = [&](int index) {
            if (index < 0) return false;
            godot::Ref<godot::Material> mat = mesh->get_active_material(index);
            godot::Ref<godot::StandardMaterial3D> std_mat = godot::Object::cast_to<godot::StandardMaterial3D>(mat.ptr());
            if (!std_mat.is_valid()) {
                if (mat.is_valid()) return false;
                std_mat = godot::Ref<godot::StandardMaterial3D>(memnew(godot::StandardMaterial3D));
                mesh->set_surface_override_material(index, std_mat);
            }
            std_mat->set_flag(static_cast<godot::BaseMaterial3D::Flags>(flag), state);
            return true;
        };
        if (Tool::contains_wildcard(material)) {
            for (const auto& name : get_materials(component)) {
                if (!Tool::match_wildcard(material, name)) continue;
                int index = find_material_index(mesh, name);
                if (index >= 0) exec(index);
            }
        }
        else if (!exec(find_material_index(mesh, material))) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: material '{}' not found in component '{}'", material, component));
        return true;
    }

    bool Model::set_blendshape_value(const std::string& component, const std::string& blend_shape, float value) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: component '{}' not found in model '{}'", component, model_name));
        auto exec = [&](const std::string& name) {
            int index = mesh -> find_blend_shape_by_name(Tool::to_godot_string(name));
            if (index < 0) return false;
            mesh -> set_blend_shape_value(index, value);
            return true;
        };
        if (Tool::contains_wildcard(blend_shape)) {
            for (const auto& name : get_blendshapes(component)) {
                if (Tool::match_wildcard(blend_shape, name)) exec(name);
            }
        }
        else if (!exec(blend_shape)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: blendshape '{}' not found in component '{}'", blend_shape, component));
        return true;
    }

    void Model::set_animation_speed(float speed) {
        if (!animation_player) return;
        animation_player->set_speed_scale(speed);
    }

    void Model::set_sync_authority(int peer_id) {
        if (!net_sync) return;
        net_sync->set_multiplayer_authority(peer_id);
    }


    // Getters //
    Model::Format Model::get_format(const godot::PackedByteArray& buffer) {
        const uint8_t* ptr = buffer.ptr();
        const int size = buffer.size();
        if (size >= 4 &&
            ptr[0] == 0x67 && ptr[1] == 0x6C &&
            ptr[2] == 0x54 && ptr[3] == 0x46) return Format::GLB;
        return Format::UNKNOWN;
    }

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
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: component '{}' not found in model '{}'", component, model_name));
        std::vector<std::string> materials;
        godot::ArrayMesh* array_mesh = godot::Object::cast_to<godot::ArrayMesh>(mesh -> get_mesh().ptr());
        if (!array_mesh) return materials;
        for (int i = 0; i < array_mesh->get_surface_count(); i++) {
            materials.push_back(Tool::to_std_string(array_mesh->surface_get_name(i)));
        }
        return materials;
    }

    std::vector<std::string> Model::get_blendshapes(const std::string& component) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: component '{}' not found in model '{}'", component, model_name));
        std::vector<std::string> blendshapes;
        godot::ArrayMesh* array_mesh = godot::Object::cast_to<godot::ArrayMesh>(mesh -> get_mesh().ptr());
        if (!array_mesh) return blendshapes;
        for (int i = 0; i < mesh -> get_blend_shape_count(); i++) {
            blendshapes.push_back(Tool::to_std_string(array_mesh->get_blend_shape_name(i)));
        }
        return blendshapes;
    }

    std::vector<std::string> Model::get_bones() {
        std::vector<std::string> bones;
        if (skeleton) {
            for (int i = 0; i < skeleton->get_bone_count(); i++) {
                bones.push_back(Tool::to_std_string(skeleton->get_bone_name(i)));
            }
        }
        return bones;
    }

    std::vector<std::string> Model::get_animations() {
        std::vector<std::string> animations;
        if (animation_player) {
            auto animation_list = animation_player->get_animation_list();
            for (int i = 0; i < animation_list.size(); i++) {
                animations.push_back(Tool::to_std_string(animation_list[i]));
            }
        }
        return animations;
    }

    float Model::get_blendshape_value(const std::string& component, const std::string& blend_shape) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: component '{}' not found in model '{}'", component, model_name));
        int index = mesh -> find_blend_shape_by_name(Tool::to_godot_string(blend_shape));
        if (index < 0) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: blendshape '{}' not found in component '{}'", blend_shape, component));
        return mesh -> get_blend_shape_value(index);
    }

    godot::Vector3 Model::get_bone_position(const std::string& bone) {
        if (!skeleton) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: no skeleton found in model '{}'", model_name));
        int index = skeleton->find_bone(Tool::to_godot_string(bone));
        if (index == -1) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Warning, fmt::format("\n> Reason: bone '{}' not found in model '{}'", bone, model_name));
        return skeleton->get_global_transform().xform(skeleton->get_bone_global_pose(index).origin);
    }

    std::string Model::get_current_animation() {
        if (!animation_player) return "";
        return Tool::to_std_string(animation_player->get_current_animation());
    }

    float Model::get_animation_speed() {
        if (!animation_player) return 1.0f;
        return animation_player->get_speed_scale();
    }

    int Model::get_sync_authority() const {
        if (!net_sync) return 0;
        return net_sync->get_multiplayer_authority();
    }


    // APIs //
    bool Model::play_animation(const std::string& name, bool loop, float speed) {
        if (!animation_player) return false;
        if (!animation_player->has_animation(Tool::to_godot_string(name))) {
            godot::UtilityFunctions::push_warning("Animation '", Tool::to_godot_string(name), "' not found in model '", Tool::to_godot_string(model_name), "'");
            return false;
        }
        godot::Ref<godot::Animation> animation = animation_player->get_animation(Tool::to_godot_string(name));
        if (animation.is_valid()) animation->set_loop_mode(loop ? godot::Animation::LOOP_LINEAR : godot::Animation::LOOP_NONE);
        animation_player->set_speed_scale(speed);
        animation_player->play(Tool::to_godot_string(name));
        return true;
    }

    void Model::stop_animation() {
        if (!animation_player) return;
        animation_player->stop();
    }

    void Model::pause_animation() {
        if (!animation_player) return;
        animation_player->pause();
    }

    void Model::resume_animation() {
        if (!animation_player) return;
        auto current = animation_player->get_current_animation();
        if (current.is_empty()) return;
        animation_player->play(current);
    }
}