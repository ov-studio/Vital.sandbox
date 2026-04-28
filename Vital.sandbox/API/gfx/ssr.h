/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: ssr.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SSR APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>


//////////////////////
// Vital: API: SSR //
//////////////////////

namespace Vital::Sandbox::API {
    struct SSR : vm_module {
        inline static const std::string base_name = "ssr";
        using base_class = Vital::Engine::Core;

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "is_enabled", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> is_ssr_enabled());
                return 1;
            });

            API::bind(vm, {base_name}, "set_enabled", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_ssr_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "set_max_steps", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_int(1);
                base_class::get_environment() -> set_ssr_max_steps(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_max_steps", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssr_max_steps());
                return 1;
            });

            API::bind(vm, {base_name}, "set_fade_in", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssr_fade_in(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_fade_in", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssr_fade_in());
                return 1;
            });

            API::bind(vm, {base_name}, "set_fade_out", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssr_fade_out(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_fade_out", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssr_fade_out());
                return 1;
            });

            API::bind(vm, {base_name}, "set_depth_tolerance", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssr_depth_tolerance(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_depth_tolerance", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssr_depth_tolerance());
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct SSR : vm_module {};
}
#endif