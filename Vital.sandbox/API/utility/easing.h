/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: easing.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Easing APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>


/////////////////////////
// Vital: API: Easing //
/////////////////////////

namespace Vital::Sandbox::API {
    struct Easing : vm_module {
        inline static const std::vector<std::string> base_scope = {"util", "easing"};

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "lerp", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(from, to, progress)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number);

                auto from = vm -> get_double(1);
                auto to = vm -> get_double(2);
                auto progress = vm -> get_double(3);
                vm -> push_value(from + (to - from)*progress);
                return 1;
            });

            API::bind(vm, base_scope, "linear", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::linear(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "sine_in", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::sine_in(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "sine_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::sine_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "sine_in_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::sine_in_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "quad_in", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::quad_in(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "quad_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::quad_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "quad_in_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::quad_in_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "cubic_in", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::cubic_in(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "cubic_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::cubic_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "cubic_in_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::cubic_in_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "quart_in", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::quart_in(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "quart_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::quart_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "quart_in_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::quart_in_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "quint_in", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::quint_in(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "quint_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::quint_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "quint_in_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::quint_in_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "expo_in", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::expo_in(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "expo_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::expo_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "expo_in_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::expo_in_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "circ_in", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::circ_in(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "circ_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::circ_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "circ_in_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::circ_in_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "back_in", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress, overshoot = 1.70158)")
                    .require(1, &Machine::is_number)
                    .optional(2, &Machine::is_number);

                auto overshoot = vm -> is_number(2) ? vm -> get_double(2) : 1.70158;
                vm -> push_value(Tool::Easing::back_in(vm -> get_double(1), overshoot));
                return 1;
            });

            API::bind(vm, base_scope, "back_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress, overshoot = 1.70158)")
                    .require(1, &Machine::is_number)
                    .optional(2, &Machine::is_number);

                auto overshoot = vm -> is_number(2) ? vm -> get_double(2) : 1.70158;
                vm -> push_value(Tool::Easing::back_out(vm -> get_double(1), overshoot));
                return 1;
            });

            API::bind(vm, base_scope, "back_in_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress, overshoot = 1.70158)")
                    .require(1, &Machine::is_number)
                    .optional(2, &Machine::is_number);

                auto overshoot = vm -> is_number(2) ? vm -> get_double(2) : 1.70158;
                vm -> push_value(Tool::Easing::back_in_out(vm -> get_double(1), overshoot));
                return 1;
            });

            API::bind(vm, base_scope, "elastic_in", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress, amplitude = 1.0, period = 0.3)")
                    .require(1, &Machine::is_number)
                    .optional(2, &Machine::is_number)
                    .optional(3, &Machine::is_number);

                auto amplitude = vm -> is_number(2) ? vm -> get_double(2) : 1.0;
                auto period = vm -> is_number(3) ? vm -> get_double(3) : 0.3;
                vm -> push_value(Tool::Easing::elastic_in(vm -> get_double(1), amplitude, period));
                return 1;
            });

            API::bind(vm, base_scope, "elastic_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress, amplitude = 1.0, period = 0.3)")
                    .require(1, &Machine::is_number)
                    .optional(2, &Machine::is_number)
                    .optional(3, &Machine::is_number);

                auto amplitude = vm -> is_number(2) ? vm -> get_double(2) : 1.0;
                auto period = vm -> is_number(3) ? vm -> get_double(3) : 0.3;
                vm -> push_value(Tool::Easing::elastic_out(vm -> get_double(1), amplitude, period));
                return 1;
            });

            API::bind(vm, base_scope, "elastic_in_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress, amplitude = 1.0, period = 0.45)")
                    .require(1, &Machine::is_number)
                    .optional(2, &Machine::is_number)
                    .optional(3, &Machine::is_number);

                auto amplitude = vm -> is_number(2) ? vm -> get_double(2) : 1.0;
                auto period = vm -> is_number(3) ? vm -> get_double(3) : 0.45;
                vm -> push_value(Tool::Easing::elastic_in_out(vm -> get_double(1), amplitude, period));
                return 1;
            });

            API::bind(vm, base_scope, "bounce_in", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::bounce_in(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "bounce_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::bounce_out(vm -> get_double(1)));
                return 1;
            });

            API::bind(vm, base_scope, "bounce_in_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(progress)").require(1, &Machine::is_number);
                vm -> push_value(Tool::Easing::bounce_in_out(vm -> get_double(1)));
                return 1;
            });
        }
    };
}