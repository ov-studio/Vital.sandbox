/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: volumetric_fog.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Volumetric Fog APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>


/////////////////////////////////
// Vital: API: Volumetric_Fog //
/////////////////////////////////

namespace Vital::Sandbox::API {
    struct Volumetric_Fog : vm_module {
        inline static const std::string base_name = "volumetric_fog";
        using base_class = Vital::Engine::Core;

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "is_enabled", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> is_volumetric_fog_enabled());
                return 1;
            });

            API::bind(vm, {base_name}, "set_enabled", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_volumetric_fog_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "set_emission", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto color = vm -> get_color(1);
                base_class::get_environment() -> set_volumetric_fog_emission(color);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_emission", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_volumetric_fog_emission());
                return 1;
            });

            API::bind(vm, {base_name}, "set_albedo", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto color = vm -> get_color(1);
                base_class::get_environment() -> set_volumetric_fog_albedo(color);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_albedo", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_volumetric_fog_albedo());
                return 1;
            });

            API::bind(vm, {base_name}, "set_density", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_density(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_density", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_volumetric_fog_density());
                return 1;
            });

            API::bind(vm, {base_name}, "set_emission_energy", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_emission_energy(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_emission_energy", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_volumetric_fog_emission_energy());
                return 1;
            });

            API::bind(vm, {base_name}, "set_anisotropy", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_anisotropy(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_anisotropy", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_volumetric_fog_anisotropy());
                return 1;
            });

            API::bind(vm, {base_name}, "set_length", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_length(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_length", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_volumetric_fog_length());
                return 1;
            });

            API::bind(vm, {base_name}, "set_detail_spread", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_detail_spread(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_detail_spread", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_volumetric_fog_detail_spread());
                return 1;
            });

            API::bind(vm, {base_name}, "set_gi_inject", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_gi_inject(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_gi_inject", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_volumetric_fog_gi_inject());
                return 1;
            });

            API::bind(vm, {base_name}, "set_ambient_inject", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_ambient_inject(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_ambient_inject", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_volumetric_fog_ambient_inject());
                return 1;
            });

            API::bind(vm, {base_name}, "set_sky_affect", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_sky_affect(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_sky_affect", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_volumetric_fog_sky_affect());
                return 1;
            });

            API::bind(vm, {base_name}, "is_temporal_reprojection_enabled", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> is_volumetric_fog_temporal_reprojection_enabled());
                return 1;
            });

            API::bind(vm, {base_name}, "set_temporal_reprojection_enabled", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_volumetric_fog_temporal_reprojection_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "set_temporal_reprojection_amount", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_temporal_reprojection_amount(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_temporal_reprojection_amount", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_volumetric_fog_temporal_reprojection_amount());
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Volumetric_Fog : vm_module {};
}
#endif