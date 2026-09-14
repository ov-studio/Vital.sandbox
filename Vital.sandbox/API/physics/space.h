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


////////////////////////
// Vital: API: Space //
////////////////////////

namespace Vital::Sandbox::API {
    struct Space : vm_module {
        inline static const std::vector<std::string> base_scope = {"physics", "space"};

        struct Query {
            uint32_t mask = 0xFFFFFFFF;
            int max_results = 32;
            float margin = 0.04f;
            bool collide_bodies = true;
            bool collide_areas = false;
            bool hit_from_inside = false;
            bool hit_back_faces = true;
            godot::TypedArray<godot::RID> exclude;
        };

        static void read_field(Machine* vm, const std::string& key, int idx, bool& out) {
            vm -> get_table_field(key, idx);
            if (vm -> is_bool(-1)) out = vm -> get_bool(-1);
            vm -> pop(1);
        }

        static void read_field(Machine* vm, const std::string& key, int idx, int& out) {
            vm -> get_table_field(key, idx);
            if (vm -> is_number(-1)) out = vm -> get_int(-1);
            vm -> pop(1);
        }

        static void read_field(Machine* vm, const std::string& key, int idx, float& out) {
            vm -> get_table_field(key, idx);
            if (vm -> is_number(-1)) out = vm -> get_float(-1);
            vm -> pop(1);
        }

        static void read_field(Machine* vm, const std::string& key, int idx, uint32_t& out) {
            vm -> get_table_field(key, idx);
            if (vm -> is_number(-1)) out = static_cast<uint32_t>(vm -> get_int(-1));
            vm -> pop(1);
        }

        static void read_field(Machine* vm, const std::string& key, int idx, godot::TypedArray<godot::RID>& out) {
            vm -> get_table_field(key, idx);
            if (vm -> is_table(-1)) out = Space::build_exclude(vm, -1);
            vm -> pop(1);
        }

        static Query parse_options(Machine* vm, int idx) {
            Query query;
            if (!vm -> is_table(idx)) return query;
            read_field(vm, "mask", idx, query.mask);
            read_field(vm, "max_results", idx, query.max_results);
            read_field(vm, "margin", idx, query.margin);
            read_field(vm, "collide_bodies", idx, query.collide_bodies);
            read_field(vm, "collide_areas", idx, query.collide_areas);
            read_field(vm, "hit_from_inside", idx, query.hit_from_inside);
            read_field(vm, "hit_back_faces", idx, query.hit_back_faces);
            read_field(vm, "exclude", idx, query.exclude);
            return query;
        }

        static godot::PhysicsDirectSpaceState3D* get_space_state() {
            auto core = Vital::Engine::Core::get_singleton();
            if (!core || !core -> get_viewport() || !core -> get_viewport() -> get_world_3d().is_valid()) return nullptr;
            return core -> get_viewport() -> get_world_3d() -> get_direct_space_state();
        }

        static godot::TypedArray<godot::RID> build_exclude(Machine* vm, int idx) {
            godot::TypedArray<godot::RID> exclude;
            if (!vm -> is_table(idx)) return exclude;

            auto count = vm -> get_length(idx);
            for (int i = 1; i <= count; i++) {
                vm -> get_table_field(i, idx);
                auto node = resolve_entity(vm, -1);
                auto collision_object = node ? godot::Object::cast_to<godot::CollisionObject3D>(node) : nullptr;
                if (collision_object) exclude.push_back(collision_object -> get_rid());
                vm -> pop(1);
            }
            return exclude;
        }

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
            else if (type == "sphere") {
                godot::Ref<godot::SphereShape3D> shape;
                shape.instantiate();
                shape -> set_radius(read_float("radius", 0.5f));
                return shape;
            }
            else if (type == "capsule") {
                godot::Ref<godot::CapsuleShape3D> shape;
                shape.instantiate();
                shape -> set_radius(read_float("radius", 0.4f));
                shape -> set_height(read_float("height", 1.8f));
                return shape;
            }
            else if (type == "cylinder") {
                godot::Ref<godot::CylinderShape3D> shape;
                shape.instantiate();
                shape -> set_radius(read_float("radius", 0.4f));
                shape -> set_height(read_float("height", 1.8f));
                return shape;
            }
            return nullptr;
        }

        static void push_collider(Machine* vm, const godot::Dictionary& dict) {
            if (!dict.has("collider")) { vm -> push_value(false); return; }
            auto object = static_cast<godot::Object*>(dict["collider"]);
            auto node = object ? godot::Object::cast_to<godot::Node3D>(object) : nullptr;
            if (!node || !Area::push_entity(vm, node)) vm -> push_value(false);
        }

        static godot::Node3D* resolve_entity(Machine* vm, int idx) {
            if (vm_module::is_userdata<Rigid_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Rigid_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Static_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Static_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Character_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Character_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Animatable_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Animatable_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Vehicle_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Vehicle_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Area::Instance>(vm, idx)) return vm_module::get_userdata_object<Area::Instance>(vm, idx) -> get_node();
            return nullptr;
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "raycast", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(from, to, options = {})", true)
                    .require(1, &Machine::is_vector3)
                    .require(2, &Machine::is_vector3)
                    .optional(3, &Machine::is_table);

                auto from = vm -> get_vector3(1);
                auto to = vm -> get_vector3(2);
                auto query = Space::parse_options(vm, 3);

                auto space_state = Space::get_space_state();
                if (!space_state) { vm -> push_value(false); return 1; }

                auto params = godot::PhysicsRayQueryParameters3D::create(from, to, query.mask, query.exclude);
                params -> set_collide_with_bodies(query.collide_bodies);
                params -> set_collide_with_areas(query.collide_areas);
                params -> set_hit_from_inside(query.hit_from_inside);
                params -> set_hit_back_faces(query.hit_back_faces);

                auto result = space_state -> intersect_ray(params);
                if (result.is_empty()) vm -> push_value(false);
                else {
                    vm -> create_table();
                    vm -> push_value(static_cast<godot::Vector3>(result["position"]));
                    vm -> set_table_field("position", -2);
                    vm -> push_value(static_cast<godot::Vector3>(result["normal"]));
                    vm -> set_table_field("normal", -2);
                    Space::push_collider(vm, result);
                    vm -> set_table_field("collider", -2);
                }
                return 1;
            });

            API::bind(vm, base_scope, "intersect_point", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(point, options = {})", true)
                    .require(1, &Machine::is_vector3)
                    .optional(2, &Machine::is_table);

                auto point = vm -> get_vector3(1);
                auto query = Space::parse_options(vm, 2);

                auto space_state = Space::get_space_state();
                if (!space_state) { vm -> create_table(); return 1; }

                godot::Ref<godot::PhysicsPointQueryParameters3D> params;
                params.instantiate();
                params -> set_position(point);
                params -> set_collision_mask(query.mask);
                params -> set_exclude(query.exclude);
                params -> set_collide_with_bodies(query.collide_bodies);
                params -> set_collide_with_areas(query.collide_areas);

                auto results = space_state -> intersect_point(params, query.max_results);
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
                auto query = Space::parse_options(vm, 3);

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
                params -> set_margin(query.margin);
                params -> set_collision_mask(query.mask);
                params -> set_exclude(query.exclude);
                params -> set_collide_with_bodies(query.collide_bodies);
                params -> set_collide_with_areas(query.collide_areas);

                auto results = space_state -> intersect_shape(params, query.max_results);
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
                auto query = Space::parse_options(vm, 4);

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
                params -> set_collision_mask(query.mask);
                params -> set_exclude(query.exclude);
                params -> set_collide_with_bodies(query.collide_bodies);
                params -> set_collide_with_areas(query.collide_areas);

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
