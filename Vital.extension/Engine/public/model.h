/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: model.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Model Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/index.h>


//////////////////////////
// Vital: Godot: Model //
//////////////////////////

namespace Vital::Godot {
    class ModelObject : public godot::Node3D {
        GDCLASS(ModelObject, godot::Node3D)
        protected:
            static void _bind_methods() {};
        private:
            godot::String model_name;
            godot::AnimationPlayer* animation_player = nullptr;
            godot::AnimationPlayer* find_animation_player(Node* node);
        public:
            ModelObject() = default;
            ~ModelObject() override = default;

            void set_model_name(const godot::String& name);
            godot::String get_model_name() const;
            void set_position(float x, float y, float z);
            void set_rotation(float x, float y, float z);
            godot::Vector3 get_position_vec() const;
            godot::Vector3 get_rotation_vec() const;

            // Animation functions
            bool play_animation(const godot::String& animation_name, bool loop = true, float speed = 1.0f);
            void stop_animation();
            void pause_animation();
            void resume_animation();
            bool is_animation_playing() const;
            godot::String get_current_animation() const;
            godot::Array get_available_animations() const;
            void set_animation_speed(float speed);
            float get_animation_speed() const;

            void _ready() override;
    };

    class ModelLoader : public godot::Node {
        GDCLASS(ModelLoader, godot::Node)
        protected:
            static void _bind_methods() {};
        private:
            inline static ModelLoader* singleton = nullptr;
            std::unordered_map<std::string, godot::Ref<godot::PackedScene>> loaded_models;
            godot::Ref<godot::PackedScene> load_from_absolute_path(const godot::String& file_path);
        public:
            ModelLoader();
            ~ModelLoader();

            static ModelLoader* get_singleton();

            bool load_model(const godot::String& model_name, const godot::String& file_path);
            ModelObject* create_object(const godot::String& model_name);
            bool unload_model(const godot::String& model_name);
            bool is_model_loaded(const godot::String& model_name) const;
            godot::Array get_loaded_models() const;
    };
}