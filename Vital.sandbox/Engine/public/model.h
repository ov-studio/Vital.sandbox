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

            // Callback registered by the API layer so it can null out stale Lua
            // instance pointers when a Model node is freed (e.g. server-side destroy
            // replicated to clients via MultiplayerSpawner despawn).
            inline static std::function<void(Model*)> on_destroyed_callback;
        protected:
            static void _bind_methods() {};
        private:
            std::string model_name;
            int pending_authority = 1;
            bool is_placeholder = false;
            godot::Skeleton3D* skeleton = nullptr;
            godot::AnimationPlayer* anim_player = nullptr;
            godot::MultiplayerSynchronizer* net_sync = nullptr;
            inline static godot::MultiplayerSpawner* net_spawner = nullptr;
            inline static ModelSpawnerDelegate* net_spawner_delegate = nullptr;
            inline static Models cache_loaded;


            // Helpers //
            template<typename T>
            T* find_node(godot::Node* node, T*& cache) {
                if (!node || cache) return cache;
                if (auto result = godot::Object::cast_to<T>(node)) {
                    cache = result;
                    return cache;
                }
                for (int i = 0; i < node -> get_child_count(); i++) {
                    if (find_node(node -> get_child(i), cache)) break;
                }
                return cache;
            }

            template<typename F>
            bool apply_standard_material(godot::MeshInstance3D* mesh, int index, F&& exec) {
                if (index < 0) return false;
                godot::Ref<godot::Material> mat = mesh -> get_active_material(index);
                godot::Ref<godot::StandardMaterial3D> std_mat = godot::Object::cast_to<godot::StandardMaterial3D>(mat.ptr());
                if (!std_mat.is_valid()) {
                    if (mat.is_valid()) return false;
                    std_mat = godot::Ref<godot::StandardMaterial3D>(memnew(godot::StandardMaterial3D));
                    mesh -> set_surface_override_material(index, std_mat);
                }
                exec(std_mat);
                return true;
            }
        
            template<typename C, typename F>
            bool apply_wildcard(const std::string& pattern, C&& condition, F&& exec) {
                if (Tool::contains_wildcard(pattern)) {
                    for (const auto& name : condition()) {
                        if (Tool::match_wildcard(pattern, name)) exec(name);
                    }
                    return true;
                }
                return exec(pattern);
            }

            godot::MeshInstance3D* find_mesh_node(godot::Node* node, const std::string& path);
            int find_material_index(godot::MeshInstance3D* mesh, const std::string& material);
            void collect_mesh_nodes(godot::Node* node, std::vector<std::string>& out, const std::string& current_path);
            void setup_sync(int authority_peer);


            // Asserts //
            godot::MeshInstance3D* assert_component(const std::string& component);
            std::pair<godot::MeshInstance3D*, int> assert_material(const std::string& component, const std::string& material);
            godot::Skeleton3D* assert_skeleton();
            godot::AnimationPlayer* assert_animation_player();
            void assert_material_feature(int feature);
            void assert_material_flag(int flag);
        public:
            // Instantiators //
            Model() = default;
            ~Model() override = default;
            void _ready() override;
            void _notification(int what);


            // Managers //
            static bool load(const std::string& name, const std::string& path);
            static bool load_from_buffer(const std::string& name, const godot::PackedByteArray& buffer);
            static bool unload(const std::string& name);
            static Model* create(const std::string& name, int authority_peer = 1);
            static void setup_spawner();
            static void teardown_spawner();
            static void cleanup_spawned();
            static void on_connected();
            void destroy();

            #if defined(Vital_SDK_Client)
            void hydrate(int authority_peer);
            #endif
            static void load_resource_models(const std::string& resource_name, const std::vector<std::string>& files);
            static void unload_resource_models(const std::string& resource_name);


            // Checkers //
            static bool is_model_loaded(const std::string& name);
            static bool is_supported_extension(const std::string& path);
            static bool is_supported_format(const std::string& path);
            bool is_synced() const;
            bool is_component_visible(const std::string& component);
            bool is_material_visible(const std::string& component, const std::string& material);
            bool is_material_feature(const std::string& component, const std::string& material, int feature);
            bool is_material_flag(const std::string& component, const std::string& material, int flag);
            bool is_animation_playing();


            // Getters //
            static Format get_format(const std::string& path);
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


            // Setters //
            void set_model_name(const std::string& name);
            void set_position(godot::Vector3 position);
            void set_rotation(godot::Vector3 rotation);
            #if !defined(Vital_SDK_Client)
            void set_sync_authority(int peer_id);
            #endif
            bool set_component_visible(const std::string& component, bool state);
            bool set_material_visible(const std::string& component, const std::string& material, bool state);
            bool set_material_feature(const std::string& component, const std::string& material, int feature, bool state);
            bool set_material_flag(const std::string& component, const std::string& material, int flag, bool state);
            bool set_blendshape_value(const std::string& component, const std::string& blend_shape, float value);
            void set_animation_speed(float speed);


            // APIs //
            bool play_animation(const std::string& name, bool loop = true, float speed = 1.0f);
            void stop_animation();
            void pause_animation();
            void resume_animation();


            // Friends //
            friend class ModelSpawnerDelegate;
    };
}