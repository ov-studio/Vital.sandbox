/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: gfx: volumetric_fog.h
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
#include <Vital.extension/Sandbox/index.h>


//////////////////////////////////////////
// Vital: Sandbox: API: Volumetric_Fog //
//////////////////////////////////////////

namespace Vital::Sandbox::API {
    struct Volumetric_Fog : vm_module {
        inline static const std::string base_name = "gfx";
        using base_class = Vital::Engine::Core;

        static void bind(Machine* vm) {
            API::bind(vm, {base_name, "volumetric_fog"}, "is_enabled", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> is_volumetric_fog_enabled());
                return 1;
            });

            API::bind(vm, {base_name, "volumetric_fog"}, "set_enabled", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_bool(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_volumetric_fog_enabled(state);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "set_emission", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_color(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto color = vm -> get_color(1);
                base_class::get_environment() -> set_volumetric_fog_emission(color);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "get_emission", [](auto vm) -> int {
                vm -> push_color(base_class::get_environment() -> get_volumetric_fog_emission());
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "set_albedo", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_color(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto color = vm -> get_color(1);
                base_class::get_environment() -> set_volumetric_fog_albedo(color);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "get_albedo", [](auto vm) -> int {
                vm -> push_color(base_class::get_environment() -> get_volumetric_fog_albedo());
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "set_density", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_density(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "get_density", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_volumetric_fog_density());
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "set_emission_energy", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_emission_energy(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "get_emission_energy", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_volumetric_fog_emission_energy());
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "set_anisotropy", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_anisotropy(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "get_anisotropy", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_volumetric_fog_anisotropy());
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "set_length", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_length(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "get_length", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_volumetric_fog_length());
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "set_detail_spread", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_detail_spread(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "get_detail_spread", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_volumetric_fog_detail_spread());
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "set_gi_inject", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_gi_inject(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "get_gi_inject", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_volumetric_fog_gi_inject());
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "set_ambient_inject", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_ambient_inject(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "get_ambient_inject", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_volumetric_fog_ambient_inject());
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "set_sky_affect", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_sky_affect(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "get_sky_affect", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_volumetric_fog_sky_affect());
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "is_temporal_reprojection_enabled", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> is_volumetric_fog_temporal_reprojection_enabled());
                return 1;
            });

            API::bind(vm, {base_name, "volumetric_fog"}, "set_temporal_reprojection_enabled", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_bool(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_volumetric_fog_temporal_reprojection_enabled(state);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "set_temporal_reprojection_amount", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_volumetric_fog_temporal_reprojection_amount(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "volumetric_fog"}, "get_temporal_reprojection_amount", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_volumetric_fog_temporal_reprojection_amount());
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