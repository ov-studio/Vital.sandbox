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
#include <Vital.sandbox/Engine/public/syncable.h>


///////////////////////////
// Vital: Engine: Model //
///////////////////////////

// TODO: Improve
namespace Vital::Manager { 
    class Network;
}

namespace Vital::Engine {
    class Model : public godot::Node3D, public ISyncable {
        GDCLASS(Model, godot::Node3D)
        friend class Network;
        friend class Manager::Network;
        public:
            enum class Format {
                GLB,
                UNKNOWN
            };

            inline static const std::vector<Tool::Format::Descriptor<Format>> format_registry = {
                { Format::GLB, "glb", { 0x67, 0x6C, 0x54, 0x46 } }
            };

            static constexpr int ANIM_LAYER_SOFT_MAX = 16;
            using Models = std::unordered_map<std::string, godot::Ref<godot::PackedScene>>;
        private:
            std::string model_name;
            int pending_authority = 1;
            bool remote = false;
            bool placeholder = false;
            godot::Skeleton3D* skeleton = nullptr;
            godot::AnimationPlayer* anim_player = nullptr;
            godot::AnimationTree* anim_tree = nullptr;
            godot::Ref<godot::AnimationNodeBlendTree> blend_tree;

            struct AnimLayerState {
                std::string current_anim;
                float speed = 1.0f;
                bool loop = true;
                float weight = 0.0f;
                float weight_target = 0.0f;
                float weight_rate = 0.0f; 
                bool one_shot = false;
                float one_shot_remaining = 0.0f;
                bool filter_enabled = false;
                std::vector<std::string> filter_bones;
            };
            std::vector<AnimLayerState> anim_layers;

            // Sync state lives in ISyncable base class.
            inline static Models cache_loaded;
            // SHA-256 of source file at load time — used to invalidate
            // same-path updates without wiping the whole resource cache.
            inline static std::unordered_map<std::string, std::string> cache_hashes;

            
            // Asserts //
            godot::MeshInstance3D* assert_component(const std::string& component);
            std::pair<godot::MeshInstance3D*, int> assert_material(const std::string& component, const std::string& material);
            godot::Skeleton3D* assert_skeleton();
            godot::AnimationPlayer* assert_animation_player();
            void assert_material_feature(int feature);
            void assert_material_flag(int flag);


            // Instantiators //
            Model() = default;
            ~Model() override = default;
            static void _bind_methods() {}


            // Helpers //
            bool ensure_animation_layer(int layer);
            void build_animation_tree();
            void update_animation_layers(float delta);
            bool apply_play_animation_layer(int layer, const std::string& name, bool loop, float speed, float weight, float blend_time);
            void apply_stop_animation_layer(int layer, float blend_time);
            bool apply_set_animation_layer_weight(int layer, float weight, float blend_time);
            void apply_set_animation_layer_speed(int layer, float speed);
            void apply_set_animation_layer_filter(int layer, bool enabled, const std::vector<std::string>& bone_paths);
            void broadcast_animation_layer(int mode, int layer, const std::string& name, bool loop, float speed, float weight, float blend_time);
            void broadcast_animation_layer_filter(int layer, bool enabled, const std::vector<std::string>& bone_paths);

            template<typename T>
            T* find_node(godot::Node* node, T*& cache) {
                if (!node || cache) return cache;
                if (auto result = godot::Object::cast_to<T>(node)) { cache = result; return cache; }
                for (int i = 0; i < node->get_child_count(); i++) {
                    if (find_node(node->get_child(i), cache)) break;
                }
                return cache;
            }

            template<typename F>
            bool apply_standard_material(godot::MeshInstance3D* mesh, int idx, F&& exec) {
                if (idx < 0) return false;
                godot::Ref<godot::Material> mat = mesh->get_active_material(idx);
                godot::Ref<godot::StandardMaterial3D> std_mat = godot::Object::cast_to<godot::StandardMaterial3D>(mat.ptr());
                if (!std_mat.is_valid()) {
                    if (mat.is_valid()) return false;
                    std_mat = godot::Ref<godot::StandardMaterial3D>(memnew(godot::StandardMaterial3D));
                    mesh->set_surface_override_material(idx, std_mat);
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
        public:
            // Hooks //
            void _ready() override;
            void _notification(int what);
            void _process(double delta) override;

            
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

            #if defined(VSDK_Client)
            void hydrate(int authority_peer);
            #endif

            static std::vector<std::string> filter_resource_models(const std::string& resource, const std::vector<std::string>& files);
            static void load_resource_models(const std::string& resource, const std::vector<std::string>& files);
            // Keep cache warm across restarts, but drop entries no longer listed
            // (and load any new ones). Safe for production resource updates.
            static void sync_resource_models(const std::string& resource, const std::vector<std::string>& files);
            static void unload_resource_models(const std::string& resource);


            // Checkers //
            static bool is_model_loaded(const std::string& name);
            bool is_remote() const;
            bool is_streamed() const;
            bool is_component_visible(const std::string& component);
            bool is_material_visible(const std::string& component, const std::string& material);
            bool is_material_feature(const std::string& component, const std::string& material, int feature);
            bool is_material_flag(const std::string& component, const std::string& material, int flag);
            bool is_animation_playing();
            bool is_animation_layer_playing(int layer) const;


            // ISyncable interface //
            Type get_sync_type() const override { return Type::Model; }
            std::string get_sync_name() const override { return model_name; }
            bool is_sync_active() const override;
            godot::Vector3 get_sync_position() const override;
            godot::Vector3 get_sync_rotation() const override;
            godot::Vector3 get_sync_scale() const override;
            void apply_sync(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel, godot::Vector3 scale) override;
            void on_sync_process(double delta) override;
            void destroy_sync() override { this->queue_free(); }
            godot::Node3D* get_sync_node() override { return this; }

            // Getters //
            static Models get_loaded_models();
            std::string get_model_name();
            std::vector<std::string> get_components();
            std::vector<std::string> get_materials(const std::string& component);
            std::vector<std::string> get_blendshapes(const std::string& component);
            std::vector<std::string> get_bones();
            std::vector<std::string> get_animations();
            float get_blendshape_value(const std::string& component, const std::string& blend_shape);
            godot::Vector3 get_bone_position(const std::string& bone);
            std::string get_current_animation();
            float get_animation_speed();
            int get_animation_layer_count() const;
            float get_animation_layer_weight(int layer) const;
            float get_animation_layer_speed(int layer) const;
            std::string get_current_animation_layer(int layer) const;
            int get_sync_authority() const;
            uint32_t get_net_id() const;


            // Setters //
            void set_model_name(const std::string& name);
            #if !defined(VSDK_Client)
            void set_syncer(int peer_id);
            #endif

            bool set_component_visible(const std::string& component, bool state);
            bool set_material_visible(const std::string& component, const std::string& material, bool state);
            bool set_material_feature(const std::string& component, const std::string& material, int feature, bool state);
            bool set_material_flag(const std::string& component, const std::string& material, int flag, bool state);
            bool set_blendshape_value(const std::string& component, const std::string& blend_shape, float value);
            void set_animation_speed(float speed);
            void set_animation_layer_speed(int layer, float speed, bool sync = true);
            bool set_animation_layer_weight(int layer, float weight, float blend_time = 0.0f, bool sync = true);


            // Misc //
            bool play_animation_layer(int layer, const std::string& name, bool loop = true, float speed = 1.0f, float weight = 1.0f, float blend_time = 0.25f, bool sync = true);
            void stop_animation_layer(int layer, float blend_time = 0.25f, bool sync = true);
            bool set_animation_layer_filter(int layer, bool enabled, const std::vector<std::string>& bone_paths = {}, bool sync = true);
    };
}
