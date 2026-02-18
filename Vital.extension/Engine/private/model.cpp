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
    godot::AnimationPlayer* ModelObject::find_animation_player(godot::Node* node) {
        if (node == nullptr) {
            return nullptr;
        }

        // Check if this node is an AnimationPlayer
        godot::AnimationPlayer* anim_player = godot::Object::cast_to<godot::AnimationPlayer>(node);
        if (anim_player != nullptr) {
            return anim_player;
        }

        // Recursively search children
        for (int i = 0; i < node->get_child_count(); i++) {
            godot::Node* child = node->get_child(i);
            godot::AnimationPlayer* found = find_animation_player(child);
            if (found != nullptr) {
                return found;
            }
        }

        return nullptr;
    }

    void ModelObject::set_model_name(const godot::String& name) {
        model_name = name;
    }

    godot::String ModelObject::get_model_name() const {
        return model_name;
    }

    void ModelObject::set_position(float x, float y, float z) {
        set_global_position(godot::Vector3(x, y, z));
    }

    void ModelObject::set_rotation(float x, float y, float z) {
        set_rotation_degrees(godot::Vector3(x, y, z));
    }

    godot::Vector3 ModelObject::get_position_vec() const {
        return get_global_position();
    }

    godot::Vector3 ModelObject::get_rotation_vec() const {
        return get_rotation_degrees();
    }

    bool ModelObject::play_animation(const godot::String& animation_name, bool loop, float speed) {
        if (animation_player == nullptr) {
            godot::UtilityFunctions::push_warning("No AnimationPlayer found in model '", model_name, "'");
            return false;
        }

        if (!animation_player->has_animation(animation_name)) {
            godot::UtilityFunctions::push_warning("Animation '", animation_name, "' not found in model '", model_name, "'");
            return false;
        }

        // Get the animation and set loop mode
        godot::Ref<godot::Animation> anim = animation_player->get_animation(animation_name);
        if (anim.is_valid()) {
            if (loop) {
                anim->set_loop_mode(godot::Animation::LOOP_LINEAR);
            } else {
                anim->set_loop_mode(godot::Animation::LOOP_NONE);
            }
        }

        // Set speed and play
        animation_player->set_speed_scale(speed);
        animation_player->play(animation_name);

        godot::UtilityFunctions::print("Playing animation '", animation_name, "' on model '", model_name, "'");
        return true;
    }

    void ModelObject::stop_animation() {
        if (animation_player != nullptr) {
            animation_player->stop();
        }
    }

    void ModelObject::pause_animation() {
        if (animation_player != nullptr) {
            animation_player->pause();
        }
    }

    void ModelObject::resume_animation() {
        if (animation_player != nullptr) {
            godot::String current = animation_player->get_current_animation();
            if (!current.is_empty()) {
                animation_player->play(current);
            }
        }
    }

    bool ModelObject::is_animation_playing() const {
        if (animation_player == nullptr) {
            return false;
        }
        return animation_player->is_playing();
    }

    godot::String ModelObject::get_current_animation() const {
        if (animation_player == nullptr) {
            return "";
        }
        return animation_player->get_current_animation();
    }

    godot::Array ModelObject::get_available_animations() const {
        godot::Array animations;

        if (animation_player == nullptr) {
            return animations;
        }

        godot::PackedStringArray anim_list = animation_player->get_animation_list();
        for (int i = 0; i < anim_list.size(); i++) {
            animations.append(anim_list[i]);
        }

        return animations;
    }

    void ModelObject::set_animation_speed(float speed) {
        if (animation_player != nullptr) {
            animation_player->set_speed_scale(speed);
        }
    }

    float ModelObject::get_animation_speed() const {
        if (animation_player == nullptr) {
            return 1.0f;
        }
        return animation_player->get_speed_scale();
    }

    void ModelObject::_ready() {
        godot::Node3D::_ready();

        // Find the AnimationPlayer in the hierarchy
        animation_player = find_animation_player(this);

        if (animation_player != nullptr) {
            godot::UtilityFunctions::print("Found AnimationPlayer in model '", model_name, "'");

            // List available animations
            godot::Array anims = get_available_animations();
            if (anims.size() > 0) {
                godot::UtilityFunctions::print("Available animations: ", anims);

                // TODO: TESTING
                godot::String anim_name = anims[0];
                godot::UtilityFunctions::print("yes entered anim", anim_name);
                play_animation(anim_name, true, 1.0f);
                godot::UtilityFunctions::print("Playing animation: ", anim_name);
            }
        }
    }

    // ========== ModelLoader Implementation ==========

    ModelLoader::~ModelLoader() {
        loaded_models.clear();
    }

    ModelLoader* ModelLoader::get_singleton() {
        if (!singleton) singleton = memnew(ModelLoader);
        return singleton;
    }

    godot::Ref<godot::PackedScene> ModelLoader::load_from_absolute_path(const godot::String& file_path) {
        godot::String lower_path = file_path.to_lower();
    
        if (lower_path.ends_with(".glb") || lower_path.ends_with(".gltf")) {
            // Read raw bytes using your existing file utility
            auto buffer = Vital::Tool::File::read_binary(to_godot_string(get_directory()), file_path);
            if (buffer.is_empty()) {
                godot::UtilityFunctions::push_error("Failed to read binary from: ", file_path);
                return godot::Ref<godot::PackedScene>();
            }
    
            godot::Ref<godot::GLTFDocument> gltf_doc = memnew(godot::GLTFDocument);
            godot::Ref<godot::GLTFState> gltf_state = memnew(godot::GLTFState);
    
            // Feed raw bytes instead of letting Godot open the file itself
            // Later set basepath (second param to be sibling of filepath)
            godot::Error err = gltf_doc->append_from_buffer(buffer, "", gltf_state);
            if (err != godot::OK) {
                godot::UtilityFunctions::push_error("Failed to parse GLTF buffer from: ", file_path);
                return godot::Ref<godot::PackedScene>();
            }
    
            godot::Node* root = gltf_doc->generate_scene(gltf_state);
            if (root == nullptr) {
                godot::UtilityFunctions::push_error("Failed to generate scene from GLTF: ", file_path);
                return godot::Ref<godot::PackedScene>();
            }
    
            godot::Ref<godot::PackedScene> scene = memnew(godot::PackedScene);
            scene->pack(root);
            memdelete(root);
    
            return scene;
        }
    
        godot::UtilityFunctions::push_error("Unsupported file format: ", file_path);
        return godot::Ref<godot::PackedScene>();
    }

    bool ModelLoader::load_model(const godot::String& model_name, const godot::String& file_path) {
        std::string key = std::string(model_name.utf8().get_data());

        if (loaded_models.find(key) != loaded_models.end()) {
            godot::UtilityFunctions::push_warning("Model '", model_name, "' is already loaded.");
            return false;
        }

        godot::Ref<godot::PackedScene> scene = load_from_absolute_path(file_path);

        if (scene.is_null()) {
            godot::UtilityFunctions::push_error("Failed to load model from path: ", file_path);
            return false;
        }

        loaded_models[key] = scene;
        godot::UtilityFunctions::print("Model '", model_name, "' loaded successfully from ", file_path);
        return true;
    }

    ModelObject* ModelLoader::create_object(const godot::String& model_name) {
        std::string key = std::string(model_name.utf8().get_data());

        auto it = loaded_models.find(key);
        if (it == loaded_models.end()) {
            godot::UtilityFunctions::push_error("Model '", model_name, "' is not loaded. Call load_model first.");
            return nullptr;
        }

        ModelObject* obj = memnew(ModelObject);
        obj->set_model_name(model_name);

        godot::Node* instance = it->second->instantiate();
        if (instance == nullptr) {
            godot::UtilityFunctions::push_error("Failed to instantiate model '", model_name, "'");
            memdelete(obj);
            return nullptr;
        }

        obj->add_child(instance);
        instance->set_owner(obj);

        return obj;
    }

    bool ModelLoader::unload_model(const godot::String& model_name) {
        std::string key = std::string(model_name.utf8().get_data());

        auto it = loaded_models.find(key);
        if (it == loaded_models.end()) {
            godot::UtilityFunctions::push_warning("Model '", model_name, "' is not loaded.");
            return false;
        }

        loaded_models.erase(it);
        godot::UtilityFunctions::print("Model '", model_name, "' unloaded successfully.");
        return true;
    }

    bool ModelLoader::is_model_loaded(const godot::String& model_name) const {
        std::string key = std::string(model_name.utf8().get_data());
        return loaded_models.find(key) != loaded_models.end();
    }

    godot::Array ModelLoader::get_loaded_models() const {
        godot::Array result;
        for (const auto& pair : loaded_models) {
            result.append(godot::String(pair.first.c_str()));
        }
        return result;
    }
}