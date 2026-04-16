/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: model.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Model Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>


///////////////////////////
// Vital: Engine: Model //
///////////////////////////

namespace Vital::Engine {
    // TODO: Improve
    class Model;
    class ModelSpawnerDelegate : public godot::Node {
        GDCLASS(ModelSpawnerDelegate, godot::Node)
        protected:
            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("spawn", "data"), &ModelSpawnerDelegate::spawn);
            }
        public:
            godot::Object* spawn(godot::Variant data);
    };

    class Model : public godot::Node3D {
        GDCLASS(Model, godot::Node3D)
        public:
            enum class Format {
                GLB,
                UNKNOWN
            };

            using Models = std::unordered_map<std::string, godot::Ref<godot::PackedScene>>;
            using SyncedMap = std::unordered_map<std::string, Model*>;
        protected:
            static void _bind_methods() {};
        private:
            std::string model_name;
            int pending_authority = 1;
            godot::Skeleton3D* skeleton = nullptr;
            godot::AnimationPlayer* animation_player = nullptr;
            godot::MultiplayerSynchronizer* net_sync = nullptr;
            inline static godot::MultiplayerSpawner* net_spawner = nullptr;
            inline static ModelSpawnerDelegate* net_spawner_delegate = nullptr;
            inline static Models cache_loaded;
            inline static SyncedMap cache_synced;


            // Helpers //
            godot::MeshInstance3D* find_mesh_node(godot::Node* node, const std::string& path);
            int find_material_index(godot::MeshInstance3D* mesh, const std::string& material);
            godot::Skeleton3D* find_skeleton(godot::Node* node);
            godot::AnimationPlayer* find_animation_player(godot::Node* node);
            void collect_mesh_nodes(godot::Node* node, std::vector<std::string>& out, const std::string& current_path);
            void setup_sync(int authority_peer);


            // Asserts //
            godot::MeshInstance3D* assert_component(const std::string& component);
            std::pair<godot::MeshInstance3D*, int> assert_material(const std::string& component, const std::string& material);
            godot::Skeleton3D* Model::assert_skeleton();
            void assert_material_feature(int feature);
            void assert_material_flag(int flag);
        public:
            // Instantiators //
            Model() = default;
            ~Model() override = default;
            void _ready() override;


            // Managers //
            static bool load(const std::string& name, const std::string& path);
            static bool load_from_buffer(const std::string& name, const godot::PackedByteArray& buffer);
            static bool unload(const std::string& name);
            static Model* create(const std::string& name);
            static void create_synced(const std::string& name, int authority_peer = 1);
            static Model* spawn_synced(const std::string& name, int authority_peer);
            static Model* get_synced(const std::string& name);
            static void setup_spawner();
            static void teardown_spawner();
            static void cleanup_spawned();
            static void clear_synced();
            static void on_connected();
            void destroy();


            // Checkers //
            static bool is_model_loaded(const std::string& name);
            bool is_component_visible(const std::string& component);
            bool is_material_visible(const std::string& component, const std::string& material);
            bool is_material_feature(const std::string& component, const std::string& material, int feature);
            bool is_material_flag(const std::string& component, const std::string& material, int flag);
            bool is_animation_playing();
            bool is_synced() const;


            // Setters //
            void set_model_name(const std::string& name);
            void set_position(godot::Vector3 position);
            void set_rotation(godot::Vector3 rotation);
            bool set_component_visible(const std::string& component, bool state);
            bool set_material_visible(const std::string& component, const std::string& material, bool state);
            bool set_material_feature(const std::string& component, const std::string& material, int feature, bool state);
            bool set_material_flag(const std::string& component, const std::string& material, int flag, bool state);
            bool set_blendshape_value(const std::string& component, const std::string& blend_shape, float value);
            void set_animation_speed(float speed);
            void set_sync_authority(int peer_id);


            // Getters //
            static Format get_format(const godot::PackedByteArray& buffer);
            static Models get_loaded_models();
            std::string get_model_name();
            godot::Vector3 get_position();
            godot::Vector3 get_rotation();
            std::vector<std::string> get_components();
            std::vector<std::string> get_materials(const std::string& component);
            std::vector<std::string> get_blendshapes(const std::string& component);
            std::vector<std::string> get_bones();
            std::vector<std::string> get_animations();
            float get_blendshape_value(const std::string& component, const std::string& blend_shape);
            godot::Vector3 get_bone_position(const std::string& bone);
            std::string get_current_animation();
            float get_animation_speed();
            int get_sync_authority() const;


            // APIs //
            bool play_animation(const std::string& name, bool loop = true, float speed = 1.0f);
            void stop_animation();
            void pause_animation();
            void resume_animation();

            // Friends //
            friend class ModelSpawnerDelegate;
    };
}