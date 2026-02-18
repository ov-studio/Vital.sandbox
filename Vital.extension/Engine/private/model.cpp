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
        get_animation_player(this);

        if (animation_player) {
            godot::UtilityFunctions::print("Found AnimationPlayer in model '", model_name, "'");

            godot::Array anims = get_animations();
            if (anims.size() > 0) {
                godot::UtilityFunctions::print("Available animations: ", anims);

                // TODO: TESTING
                godot::String anim_name = anims[0];
                play_animation(anim_name, true, 1.0f);
                godot::UtilityFunctions::print("Playing animation: ", anim_name);
            }
        }
    }


    // Checkers //
    bool Model::is_model_loaded(const std::string& model_name) {
        return cache_loaded.find(model_name) != cache_loaded.end();
    }

    bool Model::is_animation_playing() const {
        if (!animation_player) return false;
        return animation_player -> is_playing();
    }


    // Setters //
    void Model::set_model_name(const godot::String& name) {
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

    godot::String Model::get_model_name() const {
        return model_name;
    }

    godot::Vector3 Model::get_position() const {
        return get_global_position();
    }

    godot::Vector3 Model::get_rotation() const {
        return get_rotation_degrees();
    }

    godot::Array Model::get_animations() const {
        godot::Array animations;
        if (!animation_player) return animations;
        godot::PackedStringArray anim_list = animation_player -> get_animation_list();
        for (int i = 0; i < anim_list.size(); i++) {
            animations.append(anim_list[i]);
        }
        return animations;
    }

    godot::AnimationPlayer* Model::get_animation_player(godot::Node* node) {
        if (!node) return nullptr;
        if (!animation_player) {
            for (int i = 0; i < node -> get_child_count(); i++) {
                auto* player = get_animation_player(node -> get_child(i));
                if (player) {
                    animation_player = player;
                    break;
                }
            }
        }
        return animation_player;
    }

    godot::String Model::get_current_animation() const {
        if (!animation_player) return "";
        return animation_player -> get_current_animation();
    }

    float Model::get_animation_speed() const {
        if (!animation_player) return 1.0f;
        return animation_player -> get_speed_scale();
    }


    // APIs //
    bool Model::load_model(const std::string& model_name, const std::string& path) {
        return load_model_from_buffer(
            model_name, 
            Vital::Tool::File::read_binary(to_godot_string(get_directory()), to_godot_string(path))
        );
    }

    bool Model::load_model_from_buffer(const std::string& model_name, const godot::PackedByteArray& buffer) {
        if (is_model_loaded(model_name)) {
            godot::UtilityFunctions::push_warning("Model '", to_godot_string(model_name), "' is already loaded.");
            return false;
        }

        godot::Ref<godot::PackedScene> scene;
        switch (get_format(buffer)) {
            case Format::GLB: {
                godot::Ref<godot::GLTFDocument> gltf_doc = memnew(godot::GLTFDocument);
                godot::Ref<godot::GLTFState> gltf_state = memnew(godot::GLTFState);
                godot::Error status = gltf_doc -> append_from_buffer(buffer, "", gltf_state);
                if (status != godot::OK) {
                    godot::UtilityFunctions::push_error("Failed to parse GLB buffer for '", to_godot_string(model_name), "'");
                    return false;
                }
                godot::Node* root = gltf_doc -> generate_scene(gltf_state);
                if (root == nullptr) {
                    godot::UtilityFunctions::push_error("Failed to generate scene for '", to_godot_string(model_name), "'");
                    return false;
                }
                scene = godot::Ref<godot::PackedScene>(memnew(godot::PackedScene));
                scene -> pack(root);
                memdelete(root);
                break;
            }
        }
        if (scene.is_null()) throw Vital::Error::fetch("invalid-arguments");
        cache_loaded[model_name] = scene;
        return true;
    }

    bool Model::unload_model(const std::string& model_name) {
        auto it = cache_loaded.find(model_name);
        if (it == cache_loaded.end()) {
            godot::UtilityFunctions::push_warning("Model '", to_godot_string(model_name), "' is not loaded.");
            return false;
        }
        cache_loaded.erase(it);
        godot::UtilityFunctions::print("Model '", to_godot_string(model_name), "' unloaded successfully.");
        return true;
    }

    Model* Model::create_object(const godot::String& model_name) {
        std::string key = std::string(model_name.utf8().get_data());

        auto it = cache_loaded.find(key);
        if (it == cache_loaded.end()) {
            godot::UtilityFunctions::push_error("Model '", model_name, "' is not loaded. Call load_model first.");
            return nullptr;
        }

        Model* obj = memnew(Model);
        obj -> set_model_name(model_name);
        godot::Node* instance = it -> second -> instantiate();
        if (instance == nullptr) {
            godot::UtilityFunctions::push_error("Failed to instantiate model '", model_name, "'");
            memdelete(obj);
            return nullptr;
        }
        obj -> add_child(instance);
        instance -> set_owner(obj);
        return obj;
    }

    bool Model::play_animation(const godot::String& animation_name, bool loop, float speed) {
        if (!animation_player) return false;
        if (!animation_player -> has_animation(animation_name)) {
            godot::UtilityFunctions::push_warning("Animation '", animation_name, "' not found in model '", model_name, "'");
            return false;
        }
        godot::Ref<godot::Animation> anim = animation_player -> get_animation(animation_name);
        if (anim.is_valid()) anim -> set_loop_mode(loop ? godot::Animation::LOOP_LINEAR : godot::Animation::LOOP_NONE);
        animation_player -> set_speed_scale(speed);
        animation_player -> play(animation_name);
        godot::UtilityFunctions::print("Playing animation '", animation_name, "' on model '", model_name, "'");
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
        godot::String current = animation_player -> get_current_animation();
        if (current.is_empty()) return;
        animation_player -> play(current);
    }
}