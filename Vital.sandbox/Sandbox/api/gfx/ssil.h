/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: gfx: ssil.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SSIL APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>


////////////////////////////////
// Vital: Sandbox: API: SSIL //
////////////////////////////////

namespace Vital::Sandbox::API {
    struct SSIL : vm_module {
        inline static const std::string base_name = "gfx";
        using base_class = Vital::Engine::Core;

        static void bind(Machine* vm) {
            API::bind(vm, {base_name, "ssil"}, "is_enabled", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> is_ssil_enabled());
                return 1;
            });

            API::bind(vm, {base_name, "ssil"}, "set_enabled", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_bool(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_ssil_enabled(state);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "ssil"}, "set_radius", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssil_radius(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "ssil"}, "get_radius", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssil_radius());
                return 1;
            });
        
            API::bind(vm, {base_name, "ssil"}, "set_intensity", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssil_intensity(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "ssil"}, "get_intensity", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssil_intensity());
                return 1;
            });
        
            API::bind(vm, {base_name, "ssil"}, "set_sharpness", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssil_sharpness(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "ssil"}, "get_sharpness", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssil_sharpness());
                return 1;
            });
        
            API::bind(vm, {base_name, "ssil"}, "set_normal_rejection", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ssil_normal_rejection(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "ssil"}, "get_normal_rejection", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> get_ssil_normal_rejection());
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct SSIL : vm_module {};
}
#endif