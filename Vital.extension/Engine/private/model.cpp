/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: model.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Model Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/model.h>


//////////////////////////
// Vital: Godot: Model //
//////////////////////////

namespace Vital::Godot {
    // Instantiators //
    void Model::_ready() {
        find_animation_player(this);
    }


    // Helpers //
    godot::MeshInstance3D* Model::find_mesh_node(godot::Node* node, const std::string& path) {
        if (!node) return nullptr;
        auto separator = path.find('/');
        std::string segment = (separator == std::string::npos) ? path : path.substr(0, separator);
        std::string remainder = (separator == std::string::npos) ? "" : path.substr(separator + 1);
        for (int i = 0; i < node -> get_child_count(); i++) {
            godot::Node* child = node -> get_child(i);
            std::string child_name = to_std_string(child -> get_name());
            if (!child_name.empty() && child_name[0] == '@') {
                auto* result = find_mesh_node(child, path);
                if (result) return result;
                continue;
            }
            if (child_name != segment) continue;
            if (remainder.empty()) {
                auto* mesh = godot::Object::cast_to<godot::MeshInstance3D>(child);
                if (mesh) return mesh;
            } else {
                auto* result = find_mesh_node(child, remainder);
                if (result) return result;
            }
        }
        return nullptr;
    }

    void Model::collect_mesh_nodes(godot::Node* node, std::vector<std::string>& out, const std::string& current_path) {
        if (!node) return;
        for (int i = 0; i < node -> get_child_count(); i++) {
            godot::Node* child = node -> get_child(i);
            std::string child_name = to_std_string(child -> get_name());
            bool is_generated = !child_name.empty() && child_name[0] == '@';
            std::string child_path = (current_path.empty() || is_generated) ? (is_generated ? "" : child_name) : current_path + "/" + child_name;
            if (!is_generated && godot::Object::cast_to<godot::MeshInstance3D>(child)) out.push_back(child_path);
            collect_mesh_nodes(child, out, child_path);
        }
    }

    godot::AnimationPlayer* Model::find_animation_player(godot::Node* node) {
        if (!node) return nullptr;
        if (!animation_player) {
            auto result = godot::Object::cast_to<godot::AnimationPlayer>(node);
            if (result) animation_player = result;
            else {
                for (int i = 0; i < node -> get_child_count(); i++) {
                    auto result = find_animation_player(node -> get_child(i));
                    if (result) {
                        animation_player = result;
                        break;
                    }
                }
            }
        }
        return animation_player;
    }


    // Checkers //
    bool Model::is_model_loaded(const std::string& name) {
        return cache_loaded.find(name) != cache_loaded.end();
    }

    bool Model::is_component_visible(const std::string& name) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, name);
        if (!mesh) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Warning, fmt::format("Component '{}' not found in model '{}'", name, model_name));
        return mesh -> is_visible();
    }

    bool Model::is_animation_playing() {
        if (!animation_player) return false;
        return animation_player -> is_playing();
    }


    // Setters //
    void Model::set_model_name(const std::string& name) {
        model_name = name;
    }

    void Model::set_position(godot::Vector3 position) {
        set_global_position(position);
    }

    void Model::set_rotation(godot::Vector3 rotation) {
        set_rotation_degrees(rotation);
    }

    void Model::set_animation_speed(float speed) {
        if (!animation_player) return;
        animation_player -> set_speed_scale(speed);
    }

    bool Model::set_component_visible(const std::string& name, bool state) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, name);
        if (!mesh) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Warning, fmt::format("Component '{}' not found in model '{}'", name, model_name));
        mesh -> set_visible(state);
        return true;
    }


    // Getters //
    Model::Format Model::get_format(const godot::PackedByteArray& buffer) {
        const uint8_t* ptr = buffer.ptr();
        const int size = buffer.size();
        if (
            size >= 4 && 
            ptr[0] == 0x67 && ptr[1] == 0x6C && 
            ptr[2] == 0x54 && ptr[3] == 0x46
        ) return Format::GLB;
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

    std::vector<std::string> Model::get_animations() {
        std::vector<std::string> animations;
        if (animation_player) {
            auto animation_list = animation_player -> get_animation_list();
            for (int i = 0; i < animation_list.size(); i++) {
                animations.push_back(to_std_string(animation_list[i]));
            }
        }
        return animations;
    }

    std::string Model::get_current_animation() {
        if (!animation_player) return "";
        return to_std_string(animation_player -> get_current_animation());
    }

    float Model::get_animation_speed() {
        if (!animation_player) return 1.0f;
        return animation_player -> get_speed_scale();
    }

    std::vector<std::string> Model::get_component_blend_shapes(const std::string& component) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Warning, fmt::format("Component '{}' not found in model '{}'", component, model_name));
        std::vector<std::string> blend_shapes;
        // ArrayMesh holds the actual blend shape names; cast from the Mesh ref
        godot::ArrayMesh* array_mesh = godot::Object::cast_to<godot::ArrayMesh>(mesh -> get_mesh().ptr());
        if (!array_mesh) return blend_shapes;
        for (int i = 0; i < mesh -> get_blend_shape_count(); i++) {
            blend_shapes.push_back(to_std_string(array_mesh -> get_blend_shape_name(i)));
        }
        return blend_shapes;
    }

    float Model::get_blend_shape_value(const std::string& component, const std::string& blend_shape) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Warning, fmt::format("Component '{}' not found in model '{}'", component, model_name));
        int index = mesh -> find_blend_shape_by_name(to_godot_string(blend_shape));
        if (index < 0) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Warning, fmt::format("Blend shape '{}' not found in component '{}'", blend_shape, component));
        return mesh -> get_blend_shape_value(index);
    }

    bool Model::set_blend_shape_value(const std::string& component, const std::string& blend_shape, float value) {
        godot::MeshInstance3D* mesh = find_mesh_node(this, component);
        if (!mesh) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Warning, fmt::format("Component '{}' not found in model '{}'", component, model_name));
        int index = mesh -> find_blend_shape_by_name(to_godot_string(blend_shape));
        if (index < 0) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Warning, fmt::format("Blend shape '{}' not found in component '{}'", blend_shape, component));
        mesh -> set_blend_shape_value(index, value);
        return true;
    }


    // APIs //
    bool Model::load_model(const std::string& name, const std::string& path) {
        return load_model_from_buffer(
            name, 
            Vital::Tool::File::read_binary(to_godot_string(get_directory()), to_godot_string(path))
        );
    }

    bool Model::load_model_from_buffer(const std::string& name, const godot::PackedByteArray& buffer) {
        if (is_model_loaded(name)) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Warning, fmt::format("Model '{}' is already loaded", name));
        godot::Ref<godot::PackedScene> scene;
        switch (get_format(buffer)) {
            case Format::GLB: {
                godot::Ref<godot::GLTFDocument> document = memnew(godot::GLTFDocument);
                godot::Ref<godot::GLTFState> state = memnew(godot::GLTFState);
                godot::Error status = document -> append_from_buffer(buffer, "", state);
                if (status != godot::OK) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error, fmt::format("Failed to parse buffer for model '{}'", name));
                godot::Node* root = document -> generate_scene(state);
                if (root == nullptr) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error, fmt::format("Failed to generate scene for model '{}'", name));
                scene = godot::Ref<godot::PackedScene>(memnew(godot::PackedScene));
                scene -> pack(root);
                memdelete(root);
                break;
            }
        }
        if (scene.is_null()) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
        cache_loaded[name] = scene;
        return true;
    }

    bool Model::unload_model(const std::string& name) {
        auto it = cache_loaded.find(name);
        if (it == cache_loaded.end()) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Warning, fmt::format("Model '{}' isn't loaded yet", name));
        cache_loaded.erase(it);
        return true;
    }

    Model* Model::create_object(const std::string& name) {
        auto it = cache_loaded.find(name);
        if (it == cache_loaded.end()) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Warning, fmt::format("Model '{}' isn't loaded yet", name));
        Model* object = memnew(Model);
        object -> set_model_name(name);
        godot::Node* instance = it -> second -> instantiate();
        if (!instance) {
            memdelete(object);
            throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error, fmt::format("Failed to instantiate model '{}'", name));
        }
        object -> add_child(instance);
        Core::get_singleton() -> add_child(object);
        return object;
    }

    bool Model::play_animation(const std::string& name, bool loop, float speed) {
        if (!animation_player) return false;
        if (!animation_player -> has_animation(to_godot_string(name))) {
            godot::UtilityFunctions::push_warning("Animation '", to_godot_string(name), "' not found in model '", to_godot_string(model_name), "'");
            return false;
        }
        godot::Ref<godot::Animation> animation = animation_player -> get_animation(to_godot_string(name));
        if (animation.is_valid()) animation -> set_loop_mode(loop ? godot::Animation::LOOP_LINEAR : godot::Animation::LOOP_NONE);
        animation_player -> set_speed_scale(speed);
        animation_player -> play(to_godot_string(name));
        return true;
    }

    void Model::stop_animation() {
        if (!animation_player) return;
        animation_player -> stop();
    }

    void Model::pause_animation() {
        if (!animation_player) return;
        animation_player -> pause();
    }

    void Model::resume_animation() {
        if (!animation_player) return;
        auto current = animation_player -> get_current_animation();
        if (current.is_empty()) return;
        animation_player -> play(current);
    }
}