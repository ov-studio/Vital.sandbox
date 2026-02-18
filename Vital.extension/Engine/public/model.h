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
        public:
            enum class Format {
                GLB,
                UNKNOWN
            };

            using Models = std::unordered_map<std::string, godot::Ref<godot::PackedScene>>;
        protected:
            static void _bind_methods() {};
        private:
            godot::String model_name;
            godot::AnimationPlayer* animation_player = nullptr;
            inline static Models cache_loaded;
        public:
            // Instantiators //
            Model() = default;
            ~Model() override = default;
            void _ready() override;


            // Checkers //
            static bool is_model_loaded(const std::string& model_name);
            bool is_animation_playing() const;


            // Setters //
            void set_model_name(const godot::String& name);
            void set_position(godot::Vector3 position);
            void set_rotation(godot::Vector3 rotation);
            void set_animation_speed(float speed);


            // Getters //
            static Format get_format(const godot::PackedByteArray& buffer);
            static Models get_loaded_models();
            godot::String get_model_name() const;
            godot::Vector3 get_position() const;
            godot::Vector3 get_rotation() const;
            godot::Array get_animations() const;
            godot::AnimationPlayer* get_animation_player(godot::Node* node);
            godot::String get_current_animation() const;
            float get_animation_speed() const;


            // APIs //
            static bool load_model(const std::string& model_name, const std::string& path);
            static bool load_model_from_buffer(const std::string& model_name, const godot::PackedByteArray& buffer);
            static bool unload_model(const std::string& model_name);
            static Model* create_object(const godot::String& model_name);
            bool play_animation(const godot::String& animation_name, bool loop = true, float speed = 1.0f);
            void stop_animation();
            void pause_animation();
            void resume_animation();
    };
}