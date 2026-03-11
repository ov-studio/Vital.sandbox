/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: gfx: ssr.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SSR APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/index.h>


///////////////////////////////
// Vital: Sandbox: API: SSR //
///////////////////////////////

namespace Vital::Sandbox::API {
    struct SSR : vm_module {
        inline static const std::string base_name = "gfx";

        static void bind(Machine* vm) {
            #if defined(Vital_SDK_Client)
            API::bind(vm, {base_name, "ssr"}, "is_enabled", [](auto vm) -> int {
                vm -> push_bool(Vital::Engine::Core::get_environment() -> is_ssr_enabled());
                return 1;
            });

            API::bind(vm, {base_name, "ssr"}, "set_enabled", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_bool(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(1);
                Vital::Engine::Core::get_environment() -> set_ssr_enabled(state);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "ssr"}, "set_max_steps", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_int(1);
                Vital::Engine::Core::get_environment() -> set_ssr_max_steps(value);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "ssr"}, "get_max_steps", [](auto vm) -> int {
                vm -> push_number(Vital::Engine::Core::get_environment() -> get_ssr_max_steps());
                return 1;
            });
        
            API::bind(vm, {base_name, "ssr"}, "set_fade_in", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                Vital::Engine::Core::get_environment() -> set_ssr_fade_in(value);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "ssr"}, "get_fade_in", [](auto vm) -> int {
                vm -> push_number(Vital::Engine::Core::get_environment() -> get_ssr_fade_in());
                return 1;
            });
        
            API::bind(vm, {base_name, "ssr"}, "set_fade_out", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                Vital::Engine::Core::get_environment() -> set_ssr_fade_out(value);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "ssr"}, "get_fade_out", [](auto vm) -> int {
                vm -> push_number(Vital::Engine::Core::get_environment() -> get_ssr_fade_out());
                return 1;
            });
        
            API::bind(vm, {base_name, "ssr"}, "set_depth_tolerance", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                Vital::Engine::Core::get_environment() -> set_ssr_depth_tolerance(value);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "ssr"}, "get_depth_tolerance", [](auto vm) -> int {
                vm -> push_number(Vital::Engine::Core::get_environment() -> get_ssr_depth_tolerance());
                return 1;
            });
            #endif
        }
    };
}