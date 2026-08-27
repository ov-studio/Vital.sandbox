/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: physics_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Physics Body APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/API/core/collision_object.h>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/classes/physics_material.hpp>


///////////////////////////////
// Vital: API: Physics_Body //
///////////////////////////////

namespace Vital::Sandbox::API {
    struct Physics_Body {
        enum class Type {
            Rigid,
            Static,
            Character,
            Animatable
        };

        inline static const std::vector<std::pair<std::string, godot::PhysicsServer3D::BodyAxis>> axis_registry = {
            { "LINEAR_X",  godot::PhysicsServer3D::BODY_AXIS_LINEAR_X  },
            { "LINEAR_Y",  godot::PhysicsServer3D::BODY_AXIS_LINEAR_Y  },
            { "LINEAR_Z",  godot::PhysicsServer3D::BODY_AXIS_LINEAR_Z  },
            { "ANGULAR_X", godot::PhysicsServer3D::BODY_AXIS_ANGULAR_X },
            { "ANGULAR_Y", godot::PhysicsServer3D::BODY_AXIS_ANGULAR_Y },
            { "ANGULAR_Z", godot::PhysicsServer3D::BODY_AXIS_ANGULAR_Z }
        };

        template<typename Instance>
        static void bind(Machine* vm) {
            API::Collision_Object::bind<Instance>(vm);
        }

        template<typename Instance, Type body_type = Type::Static>
        static void methods(Machine* vm) {
            API::Collision_Object::methods<Instance, Collision_Object::Type::Body>(vm);

            vm_module::bind_method<Instance>(vm, "get_gravity", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> get_node() -> get_gravity());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_axis_lock", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(axis)", true)
                    .require_enum(2, axis_registry);

                auto axis = static_cast<godot::PhysicsServer3D::BodyAxis>(vm -> get_int(2));
                vm -> push_value(self -> get_node() -> get_axis_lock(axis));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_axis_lock", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(axis, lock)", true)
                    .require_enum(2, axis_registry)
                    .require(3, &Machine::is_bool);

                auto axis = static_cast<godot::PhysicsServer3D::BodyAxis>(vm -> get_int(2));
                auto lock = vm -> get_bool(3);
                self -> get_node() -> set_axis_lock(axis, lock);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "add_collision_exception_with", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(body)", true)
                    .require(2, [](Machine* vm, int idx) { return vm_module::is_userdata<typename Instance::Owner::Instance>(vm, idx); });

                auto instance = vm_module::get_userdata_object<typename Instance::Owner::Instance>(vm, 2);
                if (!instance) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "target body is not valid");
                self -> get_node() -> add_collision_exception_with(instance -> get_node());
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "remove_collision_exception_with", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(body)", true)
                    .require(2, [](Machine* vm, int idx) { return vm_module::is_userdata<typename Instance::Owner::Instance>(vm, idx); });

                auto instance = vm_module::get_userdata_object<typename Instance::Owner::Instance>(vm, 2);
                if (!instance) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "target body is not valid");
                self -> get_node() -> remove_collision_exception_with(instance -> get_node());
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "move_and_collide", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(motion, test_only = false)", true)
                    .require(2, &Machine::is_vector3)
                    .optional(3, &Machine::is_bool);

                auto motion = vm -> get_vector3(2);
                auto test_only = vm -> is_bool(3) ? vm -> get_bool(3) : false;
                auto collision = self -> get_node() -> move_and_collide(motion, test_only);
                if (!collision.is_valid()) {
                    vm -> push_value(false);
                    return 1;
                }

                vm -> create_table();
                vm -> push_value(collision -> get_position());
                vm -> set_table_field("position", -2);
                vm -> push_value(collision -> get_normal());
                vm -> set_table_field("normal", -2);
                vm -> push_value(collision -> get_travel());
                vm -> set_table_field("travel", -2);
                vm -> push_value(collision -> get_remainder());
                vm -> set_table_field("remainder", -2);
                vm -> push_value(collision -> get_depth());
                vm -> set_table_field("depth", -2);
                return 1;
            });

            if constexpr (body_type == Type::Rigid || body_type == Type::Static || body_type == Type::Animatable) {
                vm_module::bind_method<Instance>(vm, "set_physics_material", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(friction, bounce, rough = false, absorbent = false)", true)
                        .require(2, &Machine::is_number)
                        .require(3, &Machine::is_number)
                        .optional(4, &Machine::is_bool)
                        .optional(5, &Machine::is_bool);

                    auto friction = vm -> get_float(2);
                    auto bounce = vm -> get_float(3);
                    auto rough = vm -> is_bool(4) ? vm -> get_bool(4) : false;
                    auto absorbent = vm -> is_bool(5) ? vm -> get_bool(5) : false;

                    godot::Ref<godot::PhysicsMaterial> material;
                    material.instantiate();
                    material -> set_friction(friction);
                    material -> set_bounce(bounce);
                    material -> set_rough(rough);
                    material -> set_absorbent(absorbent);
                    self -> get_node() -> set_physics_material_override(material);
                    vm -> push_value(true);
                    return 1;
                });
            }

            if constexpr (body_type == Type::Static || body_type == Type::Animatable) {
                vm_module::bind_method<Instance>(vm, "get_constant_linear_velocity", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_constant_linear_velocity());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_constant_angular_velocity", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_constant_angular_velocity());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_constant_linear_velocity", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(velocity)", true)
                        .require(2, &Machine::is_vector3);

                    auto velocity = vm -> get_vector3(2);
                    self -> get_node() -> set_constant_linear_velocity(velocity);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_constant_angular_velocity", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(velocity)", true)
                        .require(2, &Machine::is_vector3);

                    auto velocity = vm -> get_vector3(2);
                    self -> get_node() -> set_constant_angular_velocity(velocity);
                    vm -> push_value(true);
                    return 1;
                });
            }

            if constexpr (body_type == Type::Animatable) {
                vm_module::bind_method<Instance>(vm, "is_sync_to_physics_enabled", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> is_sync_to_physics_enabled());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_sync_to_physics", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(state)", true)
                        .require(2, &Machine::is_bool);

                    auto state = vm -> get_bool(2);
                    self -> get_node() -> set_sync_to_physics(state);
                    vm -> push_value(true);
                    return 1;
                });
            }
        }

        template<typename Instance>
        static void inject(Machine* vm) {
            API::Collision_Object::inject<Instance>(vm);
        }
    };
}
#endif
