/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: space.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Physics Space Query APIs (raycasts, shape/point queries)
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/API/physics/rigid_body.h>
#include <Vital.sandbox/API/physics/static_body.h>
#include <Vital.sandbox/API/physics/character_body.h>
#include <Vital.sandbox/API/physics/animatable_body.h>
#include <Vital.sandbox/API/physics/vehicle_body.h>
#include <Vital.sandbox/API/physics/area.h>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/physics_shape_query_parameters3d.hpp>
#include <godot_cpp/classes/physics_point_query_parameters3d.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>


////////////////////////
// Vital: API: Space //
////////////////////////

namespace Vital::Sandbox::API {
    struct Space : vm_module {
        inline static const std::vector<std::string> base_scope = {"physics", "space"};

        // Resolves any physics body/area/vehicle API instance from userdata to its underlying Node3D. //
        static godot::Node3D* resolve_node(Machine* vm, int idx) {
            if (vm_module::is_userdata<Rigid_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Rigid_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Static_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Static_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Character_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Character_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Animatable_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Animatable_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Vehicle_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Vehicle_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Area::Instance>(vm, idx)) return vm_module::get_userdata_object<Area::Instance>(vm, idx) -> get_node();
            return nullptr;
        }

        static godot::PhysicsDirectSpaceState3D* get_space_state() {
            auto core = Vital::Engine::Core::get_singleton();
            if (!core || !core -> get_viewport() || !core -> get_viewport() -> get_world_3d().is_valid()) return nullptr;
            return core -> get_viewport() -> get_world_3d() -> get_direct_space_state();
        }

        // Builds a TypedArray<RID> from a Lua array table of body/area/vehicle instances. //
        static godot::TypedArray<godot::RID> build_exclude(Machine* vm, int idx) {
            godot::TypedArray<godot::RID> exclude;
            if (!vm -> is_table(idx)) return exclude;

            auto count = vm -> get_length(idx);
            for (int i = 1; i <= count; i++) {
                vm -> get_table_field(i, idx);
                auto node = resolve_node(vm, -1);
                auto collision_object = node ? godot::Object::cast_to<godot::CollisionObject3D>(node) : nullptr;
                if (collision_object) exclude.push_back(collision_object -> get_rid());
                vm -> pop(1);
            }
            return exclude;
        }

        // Builds a Ref<Shape3D> from a Lua table: {type = "box"/"sphere"/"capsule"/"cylinder", ...}. //
        static godot::Ref<godot::Shape3D> build_shape(Machine* vm, int idx) {
            vm -> get_table_field("type", idx);
            auto type = vm -> is_string(-1) ? vm -> get_string(-1) : std::string();
            vm -> pop(1);

            auto read_float = [&](const std::string& key, float fallback) -> float {
                vm -> get_table_field(key, idx);
                auto value = vm -> is_number(-1) ? vm -> get_float(-1) : fallback;
                vm -> pop(1);
                return value;
            };

            if (type == "box") {
                godot::Ref<godot::BoxShape3D> shape;
                shape.instantiate();
                vm -> get_table_field("size", idx);
                auto size = vm -> is_vector3(-1) ? vm -> get_vector3(-1) : godot::Vector3(1, 1, 1);
                vm -> pop(1);
                shape -> set_size(size);
                return shape;
            }

            if (type == "sphere") {
                godot::Ref<godot::SphereShape3D> shape;
                shape.instantiate();
                shape -> set_radius(read_float("radius", 0.5f));
                return shape;
            }

            if (type == "capsule") {
                godot::Ref<godot::CapsuleShape3D> shape;
                shape.instantiate();
                shape -> set_radius(read_float("radius", 0.4f));
                shape -> set_height(read_float("height", 1.8f));
                return shape;
            }

            if (type == "cylinder") {
                godot::Ref<godot::CylinderShape3D> shape;
                shape.instantiate();
                shape -> set_radius(read_float("radius", 0.4f));
                shape -> set_height(read_float("height", 1.8f));
                return shape;
            }

            return nullptr;
        }

        // Pushes whichever Lua-wrapped instance owns a "collider" Object from a query result Dictionary, or false. //
        static void push_collider(Machine* vm, const godot::Dictionary& dict) {
            if (!dict.has("collider")) { vm -> push_value(false); return; }
            auto object = static_cast<godot::Object*>(dict["collider"]);
            auto node = object ? godot::Object::cast_to<godot::Node3D>(object) : nullptr;
            if (!node || !Area::push_node_instance(vm, node)) vm -> push_value(false);
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "raycast", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(from, to, options = {})", true)
                    .require(1, &Machine::is_vector3)
                    .require(2, &Machine::is_vector3)
                    .optional(3, &Machine::is_table);

                auto from = vm -> get_vector3(1);
                auto to = vm -> get_vector3(2);
                auto has_options = vm -> is_table(3);

                uint32_t mask = 0xFFFFFFFF;
                bool collide_bodies = true, collide_areas = false, hit_from_inside = false, hit_back_faces = true;
                godot::TypedArray<godot::RID> exclude;

                if (has_options) {
                    vm -> get_table_field("mask", 3);
                    if (vm -> is_number(-1)) mask = static_cast<uint32_t>(vm -> get_int(-1));
                    vm -> pop(1);

                    vm -> get_table_field("collide_bodies", 3);
                    if (vm -> is_bool(-1)) collide_bodies = vm -> get_bool(-1);
                    vm -> pop(1);

                    vm -> get_table_field("collide_areas", 3);
                    if (vm -> is_bool(-1)) collide_areas = vm -> get_bool(-1);
                    vm -> pop(1);

                    vm -> get_table_field("hit_from_inside", 3);
                    if (vm -> is_bool(-1)) hit_from_inside = vm -> get_bool(-1);
                    vm -> pop(1);

                    vm -> get_table_field("hit_back_faces", 3);
                    if (vm -> is_bool(-1)) hit_back_faces = vm -> get_bool(-1);
                    vm -> pop(1);

                    vm -> get_table_field("exclude", 3);
                    if (vm -> is_table(-1)) exclude = Space::build_exclude(vm, -1);
                    vm -> pop(1);
                }

                auto space_state = Space::get_space_state();
                if (!space_state) { vm -> push_value(false); return 1; }

                auto params = godot::PhysicsRayQueryParameters3D::create(from, to, mask, exclude);
                params -> set_collide_with_bodies(collide_bodies);
                params -> set_collide_with_areas(collide_areas);
                params -> set_hit_from_inside(hit_from_inside);
                params -> set_hit_back_faces(hit_back_faces);

                auto result = space_state -> intersect_ray(params);
                if (result.is_empty()) { vm -> push_value(false); return 1; }

                vm -> create_table();
                vm -> push_value(static_cast<godot::Vector3>(result["position"]));
                vm -> set_table_field("position", -2);
                vm -> push_value(static_cast<godot::Vector3>(result["normal"]));
                vm -> set_table_field("normal", -2);
                Space::push_collider(vm, result);
                vm -> set_table_field("collider", -2);
                return 1;
            });

            API::bind(vm, base_scope, "intersect_point", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(point, options = {})", true)
                    .require(1, &Machine::is_vector3)
                    .optional(2, &Machine::is_table);

                auto point = vm -> get_vector3(1);
                auto has_options = vm -> is_table(2);

                uint32_t mask = 0xFFFFFFFF;
                int max_results = 32;
                bool collide_bodies = true, collide_areas = false;
                godot::TypedArray<godot::RID> exclude;

                if (has_options) {
                    vm -> get_table_field("mask", 2);
                    if (vm -> is_number(-1)) mask = static_cast<uint32_t>(vm -> get_int(-1));
                    vm -> pop(1);

                    vm -> get_table_field("max_results", 2);
                    if (vm -> is_number(-1)) max_results = vm -> get_int(-1);
                    vm -> pop(1);

                    vm -> get_table_field("collide_bodies", 2);
                    if (vm -> is_bool(-1)) collide_bodies = vm -> get_bool(-1);
                    vm -> pop(1);

                    vm -> get_table_field("collide_areas", 2);
                    if (vm -> is_bool(-1)) collide_areas = vm -> get_bool(-1);
                    vm -> pop(1);

                    vm -> get_table_field("exclude", 2);
                    if (vm -> is_table(-1)) exclude = Space::build_exclude(vm, -1);
                    vm -> pop(1);
                }

                auto space_state = Space::get_space_state();
                if (!space_state) { vm -> create_table(); return 1; }

                godot::Ref<godot::PhysicsPointQueryParameters3D> params;
                params.instantiate();
                params -> set_position(point);
                params -> set_collision_mask(mask);
                params -> set_exclude(exclude);
                params -> set_collide_with_bodies(collide_bodies);
                params -> set_collide_with_areas(collide_areas);

                auto results = space_state -> intersect_point(params, max_results);
                vm -> create_table();
                for (int i = 0; i < results.size(); i++) {
                    godot::Dictionary entry = results[i];
                    vm -> create_table();
                    Space::push_collider(vm, entry);
                    vm -> set_table_field("collider", -2);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            API::bind(vm, base_scope, "intersect_shape", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(shape, position, options = {})", true)
                    .require(1, &Machine::is_table)
                    .require(2, &Machine::is_vector3)
                    .optional(3, &Machine::is_table);

                auto shape = Space::build_shape(vm, 1);
                auto position = vm -> get_vector3(2);
                auto has_options = vm -> is_table(3);

                uint32_t mask = 0xFFFFFFFF;
                int max_results = 32;
                float margin = 0.04f;
                bool collide_bodies = true, collide_areas = false;
                godot::TypedArray<godot::RID> exclude;

                if (has_options) {
                    vm -> get_table_field("mask", 3);
                    if (vm -> is_number(-1)) mask = static_cast<uint32_t>(vm -> get_int(-1));
                    vm -> pop(1);

                    vm -> get_table_field("max_results", 3);
                    if (vm -> is_number(-1)) max_results = vm -> get_int(-1);
                    vm -> pop(1);

                    vm -> get_table_field("margin", 3);
                    if (vm -> is_number(-1)) margin = vm -> get_float(-1);
                    vm -> pop(1);

                    vm -> get_table_field("collide_bodies", 3);
                    if (vm -> is_bool(-1)) collide_bodies = vm -> get_bool(-1);
                    vm -> pop(1);

                    vm -> get_table_field("collide_areas", 3);
                    if (vm -> is_bool(-1)) collide_areas = vm -> get_bool(-1);
                    vm -> pop(1);

                    vm -> get_table_field("exclude", 3);
                    if (vm -> is_table(-1)) exclude = Space::build_exclude(vm, -1);
                    vm -> pop(1);
                }

                vm -> create_table();
                if (!shape.is_valid()) return 1;

                auto space_state = Space::get_space_state();
                if (!space_state) return 1;

                godot::Ref<godot::PhysicsShapeQueryParameters3D> params;
                params.instantiate();
                params -> set_shape(shape);
                godot::Transform3D transform;
                transform.origin = position;
                params -> set_transform(transform);
                params -> set_margin(margin);
                params -> set_collision_mask(mask);
                params -> set_exclude(exclude);
                params -> set_collide_with_bodies(collide_bodies);
                params -> set_collide_with_areas(collide_areas);

                auto results = space_state -> intersect_shape(params, max_results);
                for (int i = 0; i < results.size(); i++) {
                    godot::Dictionary entry = results[i];
                    vm -> create_table();
                    Space::push_collider(vm, entry);
                    vm -> set_table_field("collider", -2);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            API::bind(vm, base_scope, "cast_motion", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(shape, position, motion, options = {})", true)
                    .require(1, &Machine::is_table)
                    .require(2, &Machine::is_vector3)
                    .require(3, &Machine::is_vector3)
                    .optional(4, &Machine::is_table);

                auto shape = Space::build_shape(vm, 1);
                auto position = vm -> get_vector3(2);
                auto motion = vm -> get_vector3(3);
                auto has_options = vm -> is_table(4);

                uint32_t mask = 0xFFFFFFFF;
                bool collide_bodies = true, collide_areas = false;
                godot::TypedArray<godot::RID> exclude;

                if (has_options) {
                    vm -> get_table_field("mask", 4);
                    if (vm -> is_number(-1)) mask = static_cast<uint32_t>(vm -> get_int(-1));
                    vm -> pop(1);

                    vm -> get_table_field("collide_bodies", 4);
                    if (vm -> is_bool(-1)) collide_bodies = vm -> get_bool(-1);
                    vm -> pop(1);

                    vm -> get_table_field("collide_areas", 4);
                    if (vm -> is_bool(-1)) collide_areas = vm -> get_bool(-1);
                    vm -> pop(1);

                    vm -> get_table_field("exclude", 4);
                    if (vm -> is_table(-1)) exclude = Space::build_exclude(vm, -1);
                    vm -> pop(1);
                }

                if (!shape.is_valid()) { vm -> push_value(false); return 1; }

                auto space_state = Space::get_space_state();
                if (!space_state) { vm -> push_value(false); return 1; }

                godot::Ref<godot::PhysicsShapeQueryParameters3D> params;
                params.instantiate();
                params -> set_shape(shape);
                godot::Transform3D transform;
                transform.origin = position;
                params -> set_transform(transform);
                params -> set_motion(motion);
                params -> set_collision_mask(mask);
                params -> set_exclude(exclude);
                params -> set_collide_with_bodies(collide_bodies);
                params -> set_collide_with_areas(collide_areas);

                auto safety = space_state -> cast_motion(params);
                if (safety.size() < 2) { vm -> push_value(false); return 1; }

                vm -> create_table();
                vm -> push_value(safety[0]);
                vm -> set_table_field("safe", -2);
                vm -> push_value(safety[1]);
                vm -> set_table_field("unsafe", -2);
                return 1;
            });
        }
    };
}
