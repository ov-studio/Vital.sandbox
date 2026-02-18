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
    // Loaders //
    bool Model::is_model_loaded(const godot::String& model_name) {
        std::string key = std::string(model_name.utf8().get_data());
        return loaded_models.find(key) != loaded_models.end();
    }

    bool Model::load_model(const godot::String& model_name, const godot::String& path) {
        return load_model_from_buffer(
            model_name, 
            Vital::Tool::File::read_binary(to_godot_string(get_directory()), path)
        );
    }

    bool Model::load_model_from_buffer(const godot::String& model_name, const godot::PackedByteArray& buffer) {
        if (is_model_loaded(model_name)) {
            godot::UtilityFunctions::push_warning("Model '", model_name, "' is already loaded.");
            return false;
        }

        godot::Ref<godot::PackedScene> scene;
        switch (get_format(buffer)) {
            case Format::GLB: {
                godot::Ref<godot::GLTFDocument> gltf_doc = memnew(godot::GLTFDocument);
                godot::Ref<godot::GLTFState> gltf_state = memnew(godot::GLTFState);
                godot::Error status = gltf_doc->append_from_buffer(buffer, "", gltf_state);
                if (status != godot::OK) {
                    godot::UtilityFunctions::push_error("Failed to parse GLB buffer for '", model_name, "'");
                    return false;
                }
                godot::Node* root = gltf_doc->generate_scene(gltf_state);
                if (root == nullptr) {
                    godot::UtilityFunctions::push_error("Failed to generate scene for '", model_name, "'");
                    return false;
                }
                scene = godot::Ref<godot::PackedScene>(memnew(godot::PackedScene));
                scene->pack(root);
                memdelete(root);
                break;
            }
        }
        if (scene.is_null()) throw Vital::Error::fetch("invalid-arguments");
        std::string key = std::string(model_name.utf8().get_data());
        loaded_models[key] = scene;
        return true;
    }


    bool Model::unload_model(const godot::String& model_name) {
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

    godot::Array Model::get_loaded_models() {
        godot::Array result;
        for (const auto& pair : loaded_models) {
            result.append(godot::String(pair.first.c_str()));
        }
        return result;
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


    // Getters //
    godot::String Model::get_model_name() const {
        return model_name;
    }

    godot::Vector3 Model::get_position() const {
        return get_global_position();
    }

    godot::Vector3 Model::get_rotation() const {
        return get_rotation_degrees();
    }



    // APIs //
    Model* Model::create_object(const godot::String& model_name) {
        std::string key = std::string(model_name.utf8().get_data());

        auto it = loaded_models.find(key);
        if (it == loaded_models.end()) {
            godot::UtilityFunctions::push_error("Model '", model_name, "' is not loaded. Call load_model first.");
            return nullptr;
        }

        Model* obj = memnew(Model);
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

    godot::AnimationPlayer* Model::find_animation_player(godot::Node* node) {
        if (node == nullptr) {
            return nullptr;
        }

        godot::AnimationPlayer* anim_player = godot::Object::cast_to<godot::AnimationPlayer>(node);
        if (anim_player != nullptr) {
            return anim_player;
        }

        for (int i = 0; i < node->get_child_count(); i++) {
            godot::Node* child = node->get_child(i);
            godot::AnimationPlayer* found = find_animation_player(child);
            if (found != nullptr) {
                return found;
            }
        }

        return nullptr;
    }

    bool Model::play_animation(const godot::String& animation_name, bool loop, float speed) {
        if (animation_player == nullptr) {
            godot::UtilityFunctions::push_warning("No AnimationPlayer found in model '", model_name, "'");
            return false;
        }

        if (!animation_player->has_animation(animation_name)) {
            godot::UtilityFunctions::push_warning("Animation '", animation_name, "' not found in model '", model_name, "'");
            return false;
        }

        godot::Ref<godot::Animation> anim = animation_player->get_animation(animation_name);
        if (anim.is_valid()) {
            anim->set_loop_mode(loop ? godot::Animation::LOOP_LINEAR : godot::Animation::LOOP_NONE);
        }

        animation_player->set_speed_scale(speed);
        animation_player->play(animation_name);

        godot::UtilityFunctions::print("Playing animation '", animation_name, "' on model '", model_name, "'");
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
        godot::String current = animation_player->get_current_animation();
        if (!current.is_empty()) {
            animation_player->play(current);
        }
    }

    bool Model::is_animation_playing() const {
        if (!animation_player) return false;
        return animation_player -> is_playing();
    }

    godot::String Model::get_current_animation() const {
        if (!animation_player) return "";
        return animation_player->get_current_animation();
    }

    godot::Array Model::get_available_animations() const {
        godot::Array animations;
        if (!animation_player) return animations;
        godot::PackedStringArray anim_list = animation_player->get_animation_list();
        for (int i = 0; i < anim_list.size(); i++) {
            animations.append(anim_list[i]);
        }
        return animations;
    }

    void Model::set_animation_speed(float speed) {
        if (!animation_player) return;
        animation_player->set_speed_scale(speed);
    }

    float Model::get_animation_speed() const {
        if (!animation_player) return 1.0f;
        return animation_player->get_speed_scale();
    }

    void Model::_ready() {
        godot::Node3D::_ready();

        animation_player = find_animation_player(this);

        if (animation_player) {
            godot::UtilityFunctions::print("Found AnimationPlayer in model '", model_name, "'");

            godot::Array anims = get_available_animations();
            if (anims.size() > 0) {
                godot::UtilityFunctions::print("Available animations: ", anims);

                // TODO: TESTING
                godot::String anim_name = anims[0];
                play_animation(anim_name, true, 1.0f);
                godot::UtilityFunctions::print("Playing animation: ", anim_name);
            }
        }
    }

    // ========== Loader Static Implementation ==========
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
}