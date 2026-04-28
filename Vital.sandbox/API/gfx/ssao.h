/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: ssao.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SSAO APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>


///////////////////////
// Vital: API: SSAO //
///////////////////////

namespace Vital::Sandbox::API {
    struct SSAO : vm_module {
        inline static const std::string base_name = "ssao";
        using base_class = Vital::Engine::Core;

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "is_enabled", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> is_ssao_enabled());
                return 1;
            });

            API::bind(vm, {base_name}, "set_enabled", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_ssao_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "set_radius", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssao_radius(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_radius", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssao_radius());
                return 1;
            });

            API::bind(vm, {base_name}, "set_intensity", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssao_intensity(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_intensity", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssao_intensity());
                return 1;
            });

            API::bind(vm, {base_name}, "set_power", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssao_power(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_power", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssao_power());
                return 1;
            });

            API::bind(vm, {base_name}, "set_detail", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssao_detail(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_detail", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssao_detail());
                return 1;
            });

            API::bind(vm, {base_name}, "set_horizon", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssao_horizon(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_horizon", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssao_horizon());
                return 1;
            });

            API::bind(vm, {base_name}, "set_sharpness", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssao_sharpness(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_sharpness", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssao_sharpness());
                return 1;
            });

            API::bind(vm, {base_name}, "set_direct_light_affect", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssao_direct_light_affect(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_direct_light_affect", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssao_direct_light_affect());
                return 1;
            });

            API::bind(vm, {base_name}, "set_channel_affect", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssao_ao_channel_affect(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_channel_affect", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssao_ao_channel_affect());
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct SSAO : vm_module {};
}
#endif