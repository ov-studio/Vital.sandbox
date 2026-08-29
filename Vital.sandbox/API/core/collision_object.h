/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: collision_object.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Collision Object APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/API/core/node_3d.h>


///////////////////////////////////
// Vital: API: Collision_Object //
///////////////////////////////////

namespace Vital::Sandbox::API {
    struct Collision_Object {
        enum class Type {
            Body,
            Area
        };

        template<typename Instance, Type object_type = Type::Body>
        static void bind(Machine* vm) {
            API::Node_3D::bind<Instance, Node_3D::Type::Spatial>(vm);
        }

        template<typename Instance, Type object_type = Type::Body>
        static void methods(Machine* vm) {
            API::Node_3D::methods<Instance, Node_3D::Type::Spatial>(vm);

            vm_module::bind_method<Instance>(vm, "get_collision_layer", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(static_cast<int>(self -> get_node() -> get_collision_layer()));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_collision_mask", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(static_cast<int>(self -> get_node() -> get_collision_mask()));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_collision_layer_value", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer_number)", true)
                    .require(2, &Machine::is_number);

                auto layer_number = vm -> get_int(2);
                vm -> push_value(self -> get_node() -> get_collision_layer_value(layer_number));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_collision_mask_value", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer_number)", true)
                    .require(2, &Machine::is_number);

                auto layer_number = vm -> get_int(2);
                vm -> push_value(self -> get_node() -> get_collision_mask_value(layer_number));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_collision_layer", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer)", true)
                    .require(2, &Machine::is_number);

                auto layer = vm -> get_int(2);
                self -> get_node() -> set_collision_layer(layer);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_collision_mask", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mask)", true)
                    .require(2, &Machine::is_number);

                auto mask = vm -> get_int(2);
                self -> get_node() -> set_collision_mask(mask);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_collision_layer_value", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer_number, value)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_bool);

                auto layer_number = vm -> get_int(2);
                auto value = vm -> get_bool(3);
                self -> get_node() -> set_collision_layer_value(layer_number, value);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_collision_mask_value", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer_number, value)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_bool);

                auto layer_number = vm -> get_int(2);
                auto value = vm -> get_bool(3);
                self -> get_node() -> set_collision_mask_value(layer_number, value);
                vm -> push_value(true);
                return 1;
            });

            if constexpr (object_type == Type::Body) {
                vm_module::bind_method<Instance>(vm, "get_collision_priority", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_collision_priority());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_collision_priority", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(priority)", true)
                        .require(2, &Machine::is_number);

                    auto priority = vm -> get_float(2);
                    self -> get_node() -> set_collision_priority(priority);
                    vm -> push_value(true);
                    return 1;
                });
            }
        }

        template<typename Instance>
        static void inject(Machine* vm) {}
    };
}
