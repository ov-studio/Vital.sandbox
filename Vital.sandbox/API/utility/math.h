/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: math.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Math APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>


///////////////////////
// Vital: API: Math //
///////////////////////

namespace Vital::Sandbox::API {
    struct Math : vm_module {
        inline static const std::vector<std::string> base_scope = {"util", "math"};

        static void init(Machine* vm) {
            vm -> scope_move_global(base_scope, "math", true);
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "round", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value, decimals = 0)")
                    .require(1, &Machine::is_number)
                    .optional(2, &Machine::is_number);

                auto value = vm -> get_double(1);
                auto decimals = vm -> is_number(2) ? vm -> get_int(2) : 0;
                auto factor = std::pow(10.0, decimals);
                vm -> push_value(std::round(value*factor)/factor);
                return 1;
            });

            API::bind(vm, base_scope, "percent", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value, percent)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_number);

                auto value = vm -> get_double(1);
                auto percent = vm -> get_double(2);
                vm -> push_value(value*percent*0.01);
                return 1;
            });

            API::bind(vm, base_scope, "distance_2d", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(a, b)")
                    .require(1, &Machine::is_vector2)
                    .require(2, &Machine::is_vector2);

                auto a = vm -> get_vector2(1);
                auto b = vm -> get_vector2(2);
                vm -> push_value(a.distance_to(b));
                return 1;
            });

            API::bind(vm, base_scope, "distance_3d", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(a, b)")
                    .require(1, &Machine::is_vector3)
                    .require(2, &Machine::is_vector3);

                auto a = vm -> get_vector3(1);
                auto b = vm -> get_vector3(2);
                vm -> push_value(a.distance_to(b));
                return 1;
            });

            API::bind(vm, base_scope, "rotation_2d", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(a, b)")
                    .require(1, &Machine::is_vector2)
                    .require(2, &Machine::is_vector2);

                auto a = vm -> get_vector2(1);
                auto b = vm -> get_vector2(2);
                auto rotation = godot::Math::rad_to_deg((b - a).angle());
                if (rotation < 0) rotation += 360.0;
                vm -> push_value(rotation);
                return 1;
            });

            API::bind(vm, base_scope, "project_2d", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(origin, distance, rotation)")
                    .require(1, &Machine::is_vector2)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number);

                auto origin = vm -> get_vector2(1);
                auto distance = vm -> get_float(2);
                auto rotation = godot::Math::deg_to_rad(vm -> get_float(3));
                vm -> push_value(origin + godot::Vector2(std::cos(rotation), std::sin(rotation))*distance);
                return 1;
            });
        }
    };
}