/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: collision_shape.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Collision Shape APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/collision_shape.h>
#include <Vital.sandbox/API/physics/rigid_body.h>
#include <Vital.sandbox/API/physics/static_body.h>
#include <Vital.sandbox/API/physics/character_body.h>
#include <Vital.sandbox/API/physics/animatable_body.h>
#include <Vital.sandbox/API/physics/area.h>
#include <godot_cpp/classes/box_shape3d.hpp>
#include <godot_cpp/classes/sphere_shape3d.hpp>
#include <godot_cpp/classes/capsule_shape3d.hpp>
#include <godot_cpp/classes/cylinder_shape3d.hpp>
#include <godot_cpp/classes/world_boundary_shape3d.hpp>
#include <godot_cpp/classes/separation_ray_shape3d.hpp>


///////////////////////////////////////
// Vital: API: Collision_Shape //
///////////////////////////////////////

namespace Vital::Sandbox::API {
    struct Collision_Shape : vm_module {
        inline static const std::vector<std::string> base_scope = {"physics", "collision_shape"};
        using base_class = Vital::Engine::Collision_Shape;

        struct Instance : vm_instance<Instance> {
            using Owner = Collision_Shape;
            base_class* body = nullptr;

            auto get_node() {
                return body;
            }

            bool is_alive() const {
                return body ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> body) {
                    instance -> body -> destroy();
                    instance -> body = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        // Resolves any of the physics body/area API types to their underlying Node3D owner. //
        static godot::Node3D* resolve_owner(Machine* vm, int idx) {
            if (vm_module::is_userdata<Rigid_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Rigid_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Static_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Static_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Character_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Character_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Animatable_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Animatable_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Area::Instance>(vm, idx)) return vm_module::get_userdata_object<Area::Instance>(vm, idx) -> get_node();
            return nullptr;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Collision_Shape>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(owner)", true)
                    .require(1, [](Machine* vm, int idx) { return resolve_owner(vm, idx) != nullptr; });

                auto owner = resolve_owner(vm, 1);
                auto instance = Instance::init(vm);
                instance -> body = base_class::create(owner);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Node_3D::methods<Instance, Node_3D::Type::Spatial>(vm);

            vm_module::bind_method<Instance>(vm, "is_disabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_disabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_disabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_disabled(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shape_box", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(size)", true)
                    .require(2, &Machine::is_vector3);

                auto size = vm -> get_vector3(2);
                godot::Ref<godot::BoxShape3D> shape;
                shape.instantiate();
                shape -> set_size(size);
                self -> body -> set_shape(shape);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shape_sphere", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(radius)", true)
                    .require(2, &Machine::is_number);

                auto radius = vm -> get_float(2);
                godot::Ref<godot::SphereShape3D> shape;
                shape.instantiate();
                shape -> set_radius(radius);
                self -> body -> set_shape(shape);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shape_capsule", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(radius, height)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number);

                auto radius = vm -> get_float(2);
                auto height = vm -> get_float(3);
                godot::Ref<godot::CapsuleShape3D> shape;
                shape.instantiate();
                shape -> set_radius(radius);
                shape -> set_height(height);
                self -> body -> set_shape(shape);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shape_cylinder", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(radius, height)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number);

                auto radius = vm -> get_float(2);
                auto height = vm -> get_float(3);
                godot::Ref<godot::CylinderShape3D> shape;
                shape.instantiate();
                shape -> set_radius(radius);
                shape -> set_height(height);
                self -> body -> set_shape(shape);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shape_world_boundary", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(plane_normal, plane_distance = 0)", true)
                    .require(2, &Machine::is_vector3)
                    .optional(3, &Machine::is_number);

                auto normal = vm -> get_vector3(2);
                auto distance = vm -> is_number(3) ? vm -> get_float(3) : 0.f;
                godot::Ref<godot::WorldBoundaryShape3D> shape;
                shape.instantiate();
                shape -> set_plane(godot::Plane(normal, distance));
                self -> body -> set_shape(shape);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shape_separation_ray", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(length)", true)
                    .require(2, &Machine::is_number);

                auto length = vm -> get_float(2);
                godot::Ref<godot::SeparationRayShape3D> shape;
                shape.instantiate();
                shape -> set_length(length);
                self -> body -> set_shape(shape);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            API::Node_3D::inject<Instance>(vm);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Collision_Shape : vm_module {};
}
#endif
