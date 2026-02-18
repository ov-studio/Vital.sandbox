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
    class Model : public godot::Node3D {
        GDCLASS(Model, godot::Node3D)
        protected:
            static void _bind_methods() {};
        private:
            godot::String model_name;
            godot::AnimationPlayer* animation_player = nullptr;
            godot::AnimationPlayer* find_animation_player(godot::Node* node);
            inline static std::unordered_map<std::string, godot::Ref<godot::PackedScene>> loaded_models;
            static godot::Ref<godot::PackedScene> load_from_absolute_path(const godot::String& file_path);
        public:
            // Instantiators //
            Model() = default;
            ~Model() override = default;
            void _ready() override;

            // Instance API
            void set_model_name(const godot::String& name);
            godot::String get_model_name() const;
            void set_position(godot::Vector3 position);
            void set_rotation(godot::Vector3 rotation);
            godot::Vector3 get_position() const;
            godot::Vector3 get_rotation() const;

            bool play_animation(const godot::String& animation_name, bool loop = true, float speed = 1.0f);
            void stop_animation();
            void pause_animation();
            void resume_animation();
            bool is_animation_playing() const;
            godot::String get_current_animation() const;
            godot::Array get_available_animations() const;
            void set_animation_speed(float speed);
            float get_animation_speed() const;

            // Loader static API
            static bool load_model(const godot::String& model_name, const godot::String& file_path);
            static Model* create_object(const godot::String& model_name);
            static bool unload_model(const godot::String& model_name);
            static bool is_model_loaded(const godot::String& model_name);
            static godot::Array get_loaded_models();
    };
}