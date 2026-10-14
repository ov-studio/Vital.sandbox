/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: dimension.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Virtual Dimensions
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#include <unordered_map>
#include <godot_cpp/classes/animatable_body3d.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/audio_stream_player3d.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/collision_object3d.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>


/////////////////////////////////
// Vital: Engine: Dimension //
/////////////////////////////////

// TODO: Improve
// Dimension is stored as node meta on the node it was set on; descendants inherit by walking up.
// Client shows only what matches the ACTIVE camera's dimension (camera dimension is stored the
// same way, per camera). No active camera = everything hidden. Server just stores.
//
// How a non-matching entity is switched off:
//   Node3D (model, particles, lights, bodies...): force-hidden via `visible`. Only the "scope" nodes
//     are flagged (own dimension, or direct Core child); descendants follow through the tree.
//   AudioStreamPlayer3D: force-muted via volume (timeline keeps running).
//   Physics (client + server): every dimension > 0 gets its own physics space, bodies/areas are moved into
//     it, so they only collide with / trigger / get hit by queries of the same dimension. Dimension 0 and -1
//     live in the default space. Spaces are created on demand and kept.
//   Dimension -1 (ALL): the real body lives in the default space and is mirrored by a proxy body (shared
//     shapes) in every other dimension space, so it collides with entities of every dimension. Proxies
//     are static (static bodies) or kinematic (everything else) and follow the real body each physics
//     frame. Entities touching a proxy see the real node (object id attached). Areas are not mirrored.
// The user's own visible/volume is kept aside while forced and restored after, and
// set_node_visible / set_player_volume keep working meanwhile (they only update the kept value).
namespace Vital::Engine {
    struct Dimension {
        static constexpr int ALL = -1;      // visible in every dimension
        static constexpr int INHERIT = -2;  // network sentinel: drop own override, follow parent

        struct Viewer {
            bool active = false;
            int dimension = 0;
            uint64_t camera = 0;
        };

        inline static bool refreshed = false;
        inline static Viewer last_viewer;
        inline static int last_child_count = 0;
        inline static std::unordered_map<int, godot::RID> spaces;
        inline static std::unordered_map<uint64_t, std::unordered_map<int, godot::RID>> mirrors; // real body → dimension → proxy
        inline static uint64_t mirror_frame = 0;

        static bool has(godot::Node* node) {
            return node && node -> has_meta("vsdk_dimension");
        }

        static int get(godot::Node* node) {
            for (auto it = node; it; it = it -> get_parent()) {
                if (it -> has_meta("vsdk_dimension")) return (int)it -> get_meta("vsdk_dimension");
            }
            return 0;
        }

        static void clear(godot::Node* node) {
            if (node -> has_meta("vsdk_dimension")) node -> remove_meta("vsdk_dimension");
            for (int i = 0; i < node -> get_child_count(); i++) clear(node -> get_child(i));
        }

        // Currently active camera + its dimension
        static Viewer get_viewer() {
            Viewer viewer;
            #if defined(VSDK_Client)
            auto root = Vital::Engine::Core::get_scene_root();
            auto camera = root ? root -> get_camera_3d() : nullptr;
            if (camera) {
                viewer.active = true;
                viewer.dimension = get(camera);
                viewer.camera = (uint64_t)camera -> get_instance_id();
            }
            #endif
            return viewer;
        }

        static bool is_visible(int dimension, const Viewer& viewer) {
            #if defined(VSDK_Client)
            return viewer.active && (dimension == ALL || dimension == viewer.dimension);
            #else
            return true;
            #endif
        }

        // Is `dimension` visible to the active camera
        static bool is_visible(int dimension) {
            return is_visible(dimension, get_viewer());
        }

        // Physics space of a dimension (0 and -1 = the default world space)
        static godot::RID get_space(int dimension) {
            auto default_space = Vital::Engine::Core::get_singleton() -> get_viewport() -> get_world_3d() -> get_space();
            if (dimension <= 0) return default_space;
            auto it = spaces.find(dimension);
            if (it != spaces.end()) return it -> second;
            auto server = godot::PhysicsServer3D::get_singleton();
            auto space = server -> space_create();
            server -> space_set_active(space, true);
            // A new space's default area starts empty: copy the default world's environment
            for (auto param : {godot::PhysicsServer3D::AREA_PARAM_GRAVITY, godot::PhysicsServer3D::AREA_PARAM_GRAVITY_VECTOR, godot::PhysicsServer3D::AREA_PARAM_LINEAR_DAMP, godot::PhysicsServer3D::AREA_PARAM_ANGULAR_DAMP}) {
                server -> area_set_param(space, param, server -> area_get_param(default_space, param));
            }
            spaces[dimension] = space;
            return space;
        }

        // Entering the tree resets a body to the default space, so this runs again after reparents
        static void apply_space(godot::CollisionObject3D* object, int dimension) {
            if (!object -> is_inside_tree()) return;
            auto server = godot::PhysicsServer3D::get_singleton();
            auto space = get_space(dimension);
            auto rid = object -> get_rid();
            if (godot::Object::cast_to<godot::Area3D>(object)) {
                if (server -> area_get_space(rid) != space) server -> area_set_space(rid, space);
                return;
            }
            if (server -> body_get_space(rid) != space) server -> body_set_space(rid, space);
            auto id = (uint64_t)object -> get_instance_id();
            if (dimension == ALL) mirrors.try_emplace(id);
            else free_mirror(id);
        }

        // Mirroring of dimension -1 bodies //
        static void free_mirror(uint64_t id) {
            auto it = mirrors.find(id);
            if (it == mirrors.end()) return;
            auto server = godot::PhysicsServer3D::get_singleton();
            for (auto& entry : it -> second) server -> free_rid(entry.second);
            mirrors.erase(it);
        }

        static bool is_static_body(godot::CollisionObject3D* object) {
            return godot::Object::cast_to<godot::StaticBody3D>(object) && !godot::Object::cast_to<godot::AnimatableBody3D>(object);
        }

        // Shapes, shape transforms and disabled flags of proxy == real?
        static bool shapes_match(godot::PhysicsServer3D* server, godot::RID real, godot::RID proxy) {
            auto count = server -> body_get_shape_count(real);
            if (server -> body_get_shape_count(proxy) != count) return false;
            for (int i = 0; i < count; i++) {
                if (server -> body_get_shape(real, i) != server -> body_get_shape(proxy, i)) return false;
                if (server -> body_get_shape_transform(real, i) != server -> body_get_shape_transform(proxy, i)) return false;
                if (server -> body_is_shape_disabled(real, i) != server -> body_is_shape_disabled(proxy, i)) return false;
            }
            return true;
        }

        // Called every physics frame from Core::_physics_process (before the physics step)
        static void sync_mirrors() {
            if (mirrors.empty() || spaces.empty()) return;
            auto server = godot::PhysicsServer3D::get_singleton();
            mirror_frame++;
            for (auto it = mirrors.begin(); it != mirrors.end();) {
                auto object = godot::Object::cast_to<godot::CollisionObject3D>(godot::UtilityFunctions::instance_from_id((int64_t)it -> first));
                if (!object || !object -> is_inside_tree()) {
                    for (auto& entry : it -> second) server -> free_rid(entry.second);
                    it = mirrors.erase(it);
                    continue;
                }
                auto real = object -> get_rid();
                auto is_static = is_static_body(object);
                auto transform = (godot::Transform3D)server -> body_get_state(real, godot::PhysicsServer3D::BODY_STATE_TRANSFORM);
                for (auto& [dimension, space] : spaces) {
                    auto& proxy = it -> second[dimension];
                    auto created = !proxy.is_valid();
                    if (created) {
                        proxy = server -> body_create();
                        server -> body_set_mode(proxy, is_static ? godot::PhysicsServer3D::BODY_MODE_STATIC : godot::PhysicsServer3D::BODY_MODE_KINEMATIC);
                        server -> body_set_space(proxy, space);
                        server -> body_attach_object_instance_id(proxy, (uint64_t)object -> get_instance_id());
                        server -> body_set_ray_pickable(proxy, false);
                    }
                    // Static maps rarely change: full check every 30 frames, moving bodies every frame
                    if (created || !is_static || mirror_frame % 30 == 0) {
                        if (!shapes_match(server, real, proxy)) {
                            server -> body_clear_shapes(proxy);
                            for (int i = 0; i < server -> body_get_shape_count(real); i++) server -> body_add_shape(proxy, server -> body_get_shape(real, i), server -> body_get_shape_transform(real, i), server -> body_is_shape_disabled(real, i));
                        }
                        if (server -> body_get_collision_layer(proxy) != server -> body_get_collision_layer(real)) server -> body_set_collision_layer(proxy, server -> body_get_collision_layer(real));
                        if (server -> body_get_collision_mask(proxy) != server -> body_get_collision_mask(real)) server -> body_set_collision_mask(proxy, server -> body_get_collision_mask(real));
                        server -> body_set_param(proxy, godot::PhysicsServer3D::BODY_PARAM_FRICTION, server -> body_get_param(real, godot::PhysicsServer3D::BODY_PARAM_FRICTION));
                        server -> body_set_param(proxy, godot::PhysicsServer3D::BODY_PARAM_BOUNCE, server -> body_get_param(real, godot::PhysicsServer3D::BODY_PARAM_BOUNCE));
                    }
                    if ((godot::Transform3D)server -> body_get_state(proxy, godot::PhysicsServer3D::BODY_STATE_TRANSFORM) != transform) server -> body_set_state(proxy, godot::PhysicsServer3D::BODY_STATE_TRANSFORM, transform);
                }
                ++it;
            }
        }

        static void apply(godot::Node* node, const Viewer& viewer) {
            if (!node) return;
            auto dimension = get(node);
            #if defined(VSDK_Client)
            auto visible = is_visible(dimension, viewer);
            auto node_3d = godot::Object::cast_to<godot::Node3D>(node);
            if (node_3d && !godot::Object::cast_to<godot::Camera3D>(node)) {
                auto scope = has(node) || node -> get_parent() == Vital::Engine::Core::get_singleton();
                if (scope && !visible) {
                    if (!node_3d -> has_meta("vsdk_force_hidden")) {
                        node_3d -> set_meta("vsdk_user_visible", node_3d -> is_visible());
                        node_3d -> set_meta("vsdk_force_hidden", true);
                    }
                    node_3d -> set_visible(false);
                }
                else if (node_3d -> has_meta("vsdk_force_hidden")) {
                    node_3d -> set_visible((bool)node_3d -> get_meta("vsdk_user_visible"));
                    node_3d -> remove_meta("vsdk_force_hidden");
                    node_3d -> remove_meta("vsdk_user_visible");
                }
            }
            if (auto player = godot::Object::cast_to<godot::AudioStreamPlayer3D>(node)) {
                if (!visible) {
                    if (!player -> has_meta("vsdk_force_muted")) {
                        player -> set_meta("vsdk_user_volume", player -> get_volume_linear());
                        player -> set_meta("vsdk_force_muted", true);
                    }
                    player -> set_volume_linear(0.0f);
                }
                else if (player -> has_meta("vsdk_force_muted")) {
                    player -> set_volume_linear((float)player -> get_meta("vsdk_user_volume"));
                    player -> remove_meta("vsdk_force_muted");
                    player -> remove_meta("vsdk_user_volume");
                }
            }
            #endif
            if (auto object = godot::Object::cast_to<godot::CollisionObject3D>(node)) apply_space(object, dimension);
            for (int i = 0; i < node -> get_child_count(); i++) apply(node -> get_child(i), viewer);
        }

        // User-facing visible, aware of the force-hidden flag
        static bool is_node_visible(godot::Node3D* node) {
            return node -> has_meta("vsdk_force_hidden") ? (bool)node -> get_meta("vsdk_user_visible") : node -> is_visible();
        }

        // Force-hidden nodes only remember the value; it applies once they're shown again
        static void set_node_visible(godot::Node3D* node, bool visible) {
            if (node -> has_meta("vsdk_force_hidden")) node -> set_meta("vsdk_user_visible", visible);
            else node -> set_visible(visible);
        }

        static float get_player_volume(godot::AudioStreamPlayer3D* player) {
            return player -> has_meta("vsdk_force_muted") ? (float)player -> get_meta("vsdk_user_volume") : player -> get_volume_linear();
        }

        static void set_player_volume(godot::AudioStreamPlayer3D* player, float volume) {
            if (player -> has_meta("vsdk_force_muted")) player -> set_meta("vsdk_user_volume", volume);
            else player -> set_volume_linear(volume);
        }

        // Re-evaluates rendering for node + descendants. Also called from Model::hydrate().
        static void apply(godot::Node* node) {
            apply(node, get_viewer());
        }

        // Sets node's dimension and makes descendants follow it. INHERIT drops node's own override.
        static void set(godot::Node* node, int dimension) {
            clear(node);
            if (dimension != INHERIT) node -> set_meta("vsdk_dimension", dimension);
            apply(node);
        }

        // Same as set, but leaves descendants' overrides alone (deferred network replay).
        static void assign(godot::Node* node, int dimension) {
            if (dimension == INHERIT) {
                if (node -> has_meta("vsdk_dimension")) node -> remove_meta("vsdk_dimension");
            }
            else node -> set_meta("vsdk_dimension", dimension);
            apply(node);
        }

        // Cameras //
        // Only re-renders when this is the active camera (see refresh)
        static void set_camera_dimension(godot::Node* camera, int dimension) {
            set(camera, dimension);
            refresh();
        }

        // Called every frame from Core::_process and right after the active camera changes.
        // Client: full re-apply when the active camera, or its dimension, changes (incl. none).
        // Both: newly added Core children get gated / moved into their physics space.
        static void refresh() {
            auto core = Vital::Engine::Core::get_singleton();
            if (!core) return;
            auto count = core -> get_child_count();
            #if defined(VSDK_Client)
            auto viewer = get_viewer();
            if (!refreshed || viewer.active != last_viewer.active || viewer.dimension != last_viewer.dimension || viewer.camera != last_viewer.camera) {
                refreshed = true;
                last_viewer = viewer;
                last_child_count = count;
                apply(core, viewer);
                return;
            }
            #else
            Viewer viewer;
            #endif
            for (int i = last_child_count; i < count; i++) apply(core -> get_child(i), viewer);
            last_child_count = count;
        }
    };
}
