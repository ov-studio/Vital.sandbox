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

                double value = vm -> get_double(1);
                int decimals = vm -> is_number(2) ? vm -> get_int(2) : 0;
                double factor = std::pow(10.0, decimals);
                vm -> push_value(std::round(value*factor)/factor);
                return 1;
            });

            API::bind(vm, base_scope, "percent", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value, percent)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_number);

                double value = vm -> get_double(1);
                double percent = vm -> get_double(2);
                vm -> push_value(value*percent*0.01);
                return 1;
            });

            API::bind(vm, base_scope, "distance_2d", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(x1, y1, x2, y2)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number)
                    .require(4, &Machine::is_number);

                double x1 = vm -> get_double(1), y1 = vm -> get_double(2);
                double x2 = vm -> get_double(3), y2 = vm -> get_double(4);
                vm -> push_value(std::sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1)));
                return 1;
            });

            API::bind(vm, base_scope, "distance_3d", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(x1, y1, z1, x2, y2, z2)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number)
                    .require(4, &Machine::is_number)
                    .require(5, &Machine::is_number)
                    .require(6, &Machine::is_number);

                double x1 = vm -> get_double(1), y1 = vm -> get_double(2), z1 = vm -> get_double(3);
                double x2 = vm -> get_double(4), y2 = vm -> get_double(5), z2 = vm -> get_double(6);
                vm -> push_value(std::sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1) + (z2 - z1)*(z2 - z1)));
                return 1;
            });

            API::bind(vm, base_scope, "rotation_2d", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(x1, y1, x2, y2)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number)
                    .require(4, &Machine::is_number);

                double x1 = vm -> get_double(1), y1 = vm -> get_double(2);
                double x2 = vm -> get_double(3), y2 = vm -> get_double(4);
                double rotation = -godot::Math::rad_to_deg(std::atan2(x2 - x1, y2 - y1));
                if (rotation < 0) rotation += 360.0;
                vm -> push_value(rotation);
                return 1;
            });

            API::bind(vm, base_scope, "project_2d", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(x, y, distance, rotation)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number)
                    .require(4, &Machine::is_number);

                double x = vm -> get_double(1), y = vm -> get_double(2);
                double distance = vm -> get_double(3);
                double rotation = godot::Math::deg_to_rad(90.0 - vm -> get_double(4));
                vm -> push_value(x + std::cos(rotation)*distance);
                vm -> push_value(y + std::sin(rotation)*distance);
                return 2;
            });
        }
    };
}