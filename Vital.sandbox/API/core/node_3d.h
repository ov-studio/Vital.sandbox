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
            vm_module::bind_method<Instance>(vm, "set_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(position)", true)
                    .require(2, &Machine::is_vector3);

                self -> audio -> set_position(vm -> get_vector3(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_position", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_position());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_global_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(position)", true)
                    .require(2, &Machine::is_vector3);

                self -> audio -> set_global_position(vm -> get_vector3(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_global_position", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_global_position());
                return 1;
            });
            
            vm_module::bind_method<Instance>(vm, "translate", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(offset)", true)
                    .require(2, &Machine::is_vector3);

                self -> audio -> translate(vm -> get_vector3(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "translate_local", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(offset)", true)
                    .require(2, &Machine::is_vector3);

                self -> audio -> translate_object_local(vm -> get_vector3(2));
                vm -> push_value(true);
                return 1;
            });

            if constexpr (node_type == Type::Spatial) {
                vm_module::bind_method<Instance>(vm, "set_scale", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(scale)", true)
                        .require(2, &Machine::is_vector3);

                    self -> audio -> set_scale(vm -> get_vector3(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_scale", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> audio -> get_scale());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_rotation", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(euler_degrees)", true)
                        .require(2, &Machine::is_vector3);

                    self -> audio -> set_rotation_degrees(vm -> get_vector3(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_rotation", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> audio -> get_rotation_degrees());
                    return 1;
                });
                
                vm_module::bind_method<Instance>(vm, "set_global_rotation", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(euler_radians)", true)
                        .require(2, &Machine::is_vector3);

                    self -> audio -> set_global_rotation_degrees(vm -> get_vector3(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_global_rotation", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> audio -> get_global_rotation_degrees());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_rotation_degrees", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(euler_degrees)", true)
                        .require(2, &Machine::is_vector3);

                    self -> audio -> set_rotation_degrees(vm -> get_vector3(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_rotation_degrees", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> audio -> get_rotation_degrees());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "rotate_x", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(angle)", true)
                        .require(2, &Machine::is_number);

                    self -> audio -> rotate_x(vm -> get_float(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "rotate_y", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(angle)", true)
                        .require(2, &Machine::is_number);

                    self -> audio -> rotate_y(vm -> get_float(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "rotate_z", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(angle)", true)
                        .require(2, &Machine::is_number);

                    self -> audio -> rotate_z(vm -> get_float(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "look_at", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(target, up = {0,1,0})", true)
                        .require(2, &Machine::is_vector3)
                        .optional(3, &Machine::is_vector3);

                    auto target = vm -> get_vector3(2);
                    auto up     = vm -> is_vector3(3) ? vm -> get_vector3(3) : godot::Vector3(0, 1, 0);
                    self -> audio -> look_at(target, up);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "look_at_from_position", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(position, target, up = {0,1,0})", true)
                        .require(2, &Machine::is_vector3)
                        .require(3, &Machine::is_vector3)
                        .optional(4, &Machine::is_vector3);

                    auto position = vm -> get_vector3(2);
                    auto target   = vm -> get_vector3(3);
                    auto up       = vm -> is_vector3(4) ? vm -> get_vector3(4) : godot::Vector3(0, 1, 0);
                    self -> audio -> look_at_from_position(position, target, up);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "to_local", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(global_point)", true)
                        .require(2, &Machine::is_vector3);

                    vm -> push_value(self -> audio -> to_local(vm -> get_vector3(2)));
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "to_global", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(local_point)", true)
                        .require(2, &Machine::is_vector3);

                    vm -> push_value(self -> audio -> to_global(vm -> get_vector3(2)));
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "show", [](auto vm, auto self, auto& id) -> int {
                    self -> audio -> show();
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "hide", [](auto vm, auto self, auto& id) -> int {
                    self -> audio -> hide();
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_visible", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(visible)", true)
                        .require(2, &Machine::is_bool);

                    self -> audio -> set_visible(vm -> get_bool(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "is_visible", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> audio -> is_visible());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "is_visible_in_tree", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> audio -> is_visible_in_tree());
                    return 1;
                });
            }
        }

        template<typename Instance>
        static void inject(Machine* vm) {}
    };
}
#endif