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

// TOOD: Improve
namespace Vital::Manager { class Network; }

namespace Vital::Engine {
    class Model;

    class Model : public godot::Node3D {
        GDCLASS(Model, godot::Node3D)
        friend class Network;             // Engine::Network — _spawn_model/_destroy_model
        friend class Manager::Network;  // poll() lazy registration needs sync_registered
        public:
            enum class Format {
                GLB,
                UNKNOWN
            };

            inline static const std::vector<Tool::Format::Descriptor<Format>> format_registry = {
                { Format::GLB, "glb", { 0x67, 0x6C, 0x54, 0x46 } }
            };

            using Models = std::unordered_map<std::string, godot::Ref<godot::PackedScene>>;

            inline static std::function<void(Model*, bool)> on_spawned_callback;
            inline static std::function<void(Model*)> on_destroyed_callback;

            // Authority modes:
            //   authority_peer == 1  -> server-authoritative (server simulates, broadcasts)
            //   authority_peer == N  -> client-authoritative (peer N simulates, server relays)
            enum class AuthorityMode { Server, Client };

        private:
            static void _bind_methods() {}

            std::string model_name;
            int pending_authority = 1;
            bool remote = false;
            bool placeholder = false;
            godot::Skeleton3D* skeleton = nullptr;
            godot::AnimationPlayer* anim_player = nullptr;

            // ----- Low-level sync state -----
            // Authority peer (1 = server). Matches `set_syncer` semantics.
            int sync_authority = 1;

            // Last transmitted transform — used to detect drift and suppress
            // packets when the model is static.
            godot::Vector3 sync_last_pos;
            godot::Vector3 sync_last_rot;
            godot::Vector3 sync_last_vel;  // last sent/received velocity

            // Accumulated time since last broadcast tick (seconds).
            float sync_accum = 0.0f;

            // True once the model has been registered with Manager::Network.
            bool sync_registered = false;

            // When true the model is considered static and sync is suppressed.
            bool sync_sleeping = false;

            // ----- Snapshot interpolation buffer (non-authority clients only) -----
            // Stores the last N network snapshots. _process renders at
            // (current_time - BUFFER_DELAY), always interpolating between two
            // already-received states — no extrapolation in the normal case.
            static constexpr int   SNAPSHOT_COUNT = 8;
            static constexpr float BUFFER_DELAY   = 0.1f;   // 100ms render lag
            static constexpr float SNAP_THRESHOLD = 5.0f;   // units — teleport if gap > this
            static constexpr float VEL_THRESHOLD  = 0.05f;  // units/sec — "moving" cutoff

            struct Snapshot {
                godot::Vector3 pos;
                godot::Vector3 rot;
                godot::Vector3 vel;
                float          time = -1.0f; // -1 = empty slot
            };

            Snapshot snap_buf[SNAPSHOT_COUNT];
            int   snap_head  = 0;    // index of next write slot
            int   snap_count = 0;    // how many slots are filled
            float snap_clock = 0.0f; // local time counter, advanced each _process

            // Expected interval between packets — set from sync_interval on register.
            float interp_step  = 1.0f / 20.0f;
            // True once at least one snapshot has been received.
            bool  interp_ready = false;

            // Network object ID assigned by the server on creation and echoed to
            // all clients so every side refers to the same object.
            uint32_t net_id = 0;
            inline static uint32_t next_net_id = 1;

            inline static Models cache_loaded;


            // Helpers //
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

        public:
            // Hooks //
            void _ready() override;
            void _notification(int what);

            // Apply an inbound sync packet — sets interpolation target on non-authority clients.
            // Skipped entirely when this peer is the authority (raw local physics drives position).
            void apply_sync(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel);

            // Called every rendered frame — advances interpolation on non-authority clients.
            void _process(double delta) override;

            // Public raw u32 reader — used by Manager::Network to read the
            // sender_peer_id prefix in client-auth packets without exposing
            // the private read_u32 helper.
            static uint32_t read_u32_public(const godot::PackedByteArray& buf, int offset) {
                return (uint8_t)buf[offset]
                     | ((uint8_t)buf[offset+1] << 8)
                     | ((uint8_t)buf[offset+2] << 16)
                     | ((uint8_t)buf[offset+3] << 24);
            }

            // Parse a 40-byte sync entry (pos+rot+vel) from buf at byte offset.
            // Returns false if the buffer is too small.
            static bool parse_sync_packet_at(const godot::PackedByteArray& buf,
                                             int offset,
                                             uint32_t& out_id,
                                             godot::Vector3& out_pos,
                                             godot::Vector3& out_rot,
                                             godot::Vector3& out_vel);
            // Convenience wrapper (offset=0).
            static bool parse_sync_packet(const godot::PackedByteArray& buf,
                                          uint32_t& out_id,
                                          godot::Vector3& out_pos,
                                          godot::Vector3& out_rot,
                                          godot::Vector3& out_vel);


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


            // Getters //
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
            uint32_t get_net_id() const;


            // Setters //
            void set_model_name(const std::string& name);
            void set_position(godot::Vector3 position);
            void set_rotation(godot::Vector3 rotation);

            // set_syncer(peer_id) — assign authority to a client (client-auth mode).
            // set_syncer(0 or 1) — revert to server authority.
            // Server-side only. Broadcasts _set_authority to all clients so they
            // enable/disable interpolation correctly. Called automatically on disconnect.
            #if !defined(VSDK_Client)
            void set_syncer(int peer_id);
            #endif

            bool set_component_visible(const std::string& component, bool state);
            bool set_material_visible(const std::string& component, const std::string& material, bool state);
            bool set_material_feature(const std::string& component, const std::string& material, int feature, bool state);
            bool set_material_flag(const std::string& component, const std::string& material, int flag, bool state);
            bool set_blendshape_value(const std::string& component, const std::string& blend_shape, float value);
            void set_animation_speed(float speed);


            // Misc //
            bool play_animation(const std::string& name, bool loop = true, float speed = 1.0f);
            void stop_animation();
            void pause_animation();
            void resume_animation();
    };
}
