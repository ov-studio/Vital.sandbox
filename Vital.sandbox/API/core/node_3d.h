/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: node_3d.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Node 3D APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>


//////////////////////////
// Vital: API: Node 3D //
//////////////////////////

namespace Vital::Sandbox::API {
    struct Node_3D {
        enum class Type {
            Audio,
            Spatial
        };

        template<typename Instance>
        static void bind(Machine* vm) {}

        template<typename Instance, Type node_type = Type::Spatial>
        static void methods(Machine* vm) {
            if constexpr (node_type == Type::Spatial) {
                vm_module::bind_method<Instance>(vm, "is_visible", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> is_visible());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "is_visible_in_tree", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> is_visible_in_tree());
                    return 1;
                });

            }
            {
                vm_module::bind_method<Instance>(vm, "get_position", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_position());
                    return 1;
                });
    
                vm_module::bind_method<Instance>(vm, "get_global_position", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_global_position());
                    return 1;
                });
            }
            if constexpr (node_type == Type::Spatial) {
                vm_module::bind_method<Instance>(vm, "get_scale", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_scale());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_global_scale", [](auto vm, auto self, auto& id) -> int {
                    auto scale = self -> get_node() -> get_global_transform().basis.get_scale();
                    vm -> push_value(scale);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_rotation", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_rotation_degrees());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_global_rotation", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_global_rotation_degrees());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_quaternion", [](auto vm, auto self, auto& id) -> int {
                    auto quaternion = self -> get_node() -> get_quaternion();
                    auto value = godot::Vector4(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
                    vm -> push_value(value);
                    return 1;
                });
            }
            {
                vm_module::bind_method<Instance>(vm, "set_position", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(position)", true)
                        .require(2, &Machine::is_vector3);
    
                    auto position = vm -> get_vector3(2);
                    self -> get_node() -> set_position(position);
                    vm -> push_value(true);
                    return 1;
                });
    
                vm_module::bind_method<Instance>(vm, "set_global_position", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(position)", true)
                        .require(2, &Machine::is_vector3);
    
                    auto position = vm -> get_vector3(2);
                    self -> get_node() -> set_global_position(position);
                    vm -> push_value(true);
                    return 1;
                });
    
            }
            if constexpr (node_type == Type::Spatial) {
                vm_module::bind_method<Instance>(vm, "set_scale", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(scale)", true)
                        .require(2, &Machine::is_vector3);

                    auto scale = vm -> get_vector3(2);
                    self -> get_node() -> set_scale(scale);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_rotation", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(euler_degrees)", true)
                        .require(2, &Machine::is_vector3);

                    auto euler_degrees = vm -> get_vector3(2);
                    self -> get_node() -> set_rotation_degrees(euler_degrees);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_global_rotation", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(euler_degrees)", true)
                        .require(2, &Machine::is_vector3);

                    auto euler_degrees = vm -> get_vector3(2);
                    self -> get_node() -> set_global_rotation_degrees(euler_degrees);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_quaternion", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(quaternion)", true)
                        .require(2, &Machine::is_vector4);

                    auto value = vm -> get_vector4(2);
                    auto quaternion = godot::Quaternion(value.x, value.y, value.z, value.w);
                    self -> get_node() -> set_quaternion(quaternion);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_visible", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(visible)", true)
                        .require(2, &Machine::is_bool);

                    auto visible = vm -> get_bool(2);
                    self -> get_node() -> set_visible(visible);
                    vm -> push_value(true);
                    return 1;
                });
            }
            {
                vm_module::bind_method<Instance>(vm, "translate", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(offset)", true)
                        .require(2, &Machine::is_vector3);
    
                    auto offset = vm -> get_vector3(2);
                    self -> get_node() -> translate(offset);
                    vm -> push_value(true);
                    return 1;
                });
    
                vm_module::bind_method<Instance>(vm, "translate_local", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(offset)", true)
                        .require(2, &Machine::is_vector3);
    
                    auto offset = vm -> get_vector3(2);
                    self -> get_node() -> translate_object_local(offset);
                    vm -> push_value(true);
                    return 1;
                });
            }
            if constexpr (node_type == Type::Spatial) {
                vm_module::bind_method<Instance>(vm, "to_local", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(global_point)", true)
                        .require(2, &Machine::is_vector3);

                    auto global_point = vm -> get_vector3(2);
                    auto local_point = self -> get_node() -> to_local(global_point);
                    vm -> push_value(local_point);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "to_global", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(local_point)", true)
                        .require(2, &Machine::is_vector3);

                    auto local_point = vm -> get_vector3(2);
                    auto global_point = self -> get_node() -> to_global(local_point);
                    vm -> push_value(global_point);
                    return 1;
                });
                
                vm_module::bind_method<Instance>(vm, "scale_local", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(scale)", true)
                        .require(2, &Machine::is_vector3);

                    auto scale = vm -> get_vector3(2);
                    self -> get_node() -> scale_object_local(scale);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "scale_global", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(scale)", true)
                        .require(2, &Machine::is_vector3);

                    auto scale = vm -> get_vector3(2);
                    self -> get_node() -> global_scale(scale);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "rotate", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(axis, degrees)", true)
                        .require(2, &Machine::is_vector3)
                        .require(3, &Machine::is_number);

                    auto axis = vm -> get_vector3(2);
                    auto angle = vm -> get_float(3);
                    self -> get_node() -> rotate(axis, angle);
                    self -> get_node() -> rotate(axis, godot::Math::deg_to_rad(degrees));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "rotate_local", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(axis, degrees)", true)
                        .require(2, &Machine::is_vector3)
                        .require(3, &Machine::is_number);

                    auto axis = vm -> get_vector3(2);
                    auto angle = vm -> get_float(3);
                    self -> get_node() -> rotate_object_local(axis, angle);
                    auto degrees = vm -> get_float(3);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "look_at", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(target, up = {0, 1, 0})", true)
                        .require(2, &Machine::is_vector3)
                        .optional(3, &Machine::is_vector3);

                    auto target = vm -> get_vector3(2);
                    auto up = vm -> is_vector3(3) ? vm -> get_vector3(3) : godot::Vector3(0, 1, 0);
                    self -> get_node() -> look_at(target, up);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "look_at_from_position", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(position, target, up = {0, 1, 0})", true)
                        .require(2, &Machine::is_vector3)
                        .require(3, &Machine::is_vector3)
                        .optional(4, &Machine::is_vector3);

                    auto position = vm -> get_vector3(2);
                    auto target = vm -> get_vector3(3);
                    auto up = vm -> is_vector3(4) ? vm -> get_vector3(4) : godot::Vector3(0, 1, 0);
                    self -> get_node() -> look_at_from_position(position, target, up);
                    vm -> push_value(true);
                    return 1;
                });
            }
        }

        template<typename Instance>
        static void inject(Machine* vm) {}
    };
}
#endif