/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: adjustment.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Adjustment APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/index.h>


//////////////////////////////////////
// Vital: Sandbox: API: Adjustment //
//////////////////////////////////////

namespace Vital::Sandbox::API {
    struct Adjustment : vm_module {
        static void bind(Machine* vm) {            
            #if defined(Vital_SDK_Client)
            Vital::Sandbox::API::bind(vm, "adjustment", "set_enabled", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_bool(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(1);
                Vital::Godot::Core::get_environment() -> set_adjustment_enabled(state);
                vm -> push_bool(true);
                return 1;
            });
        
            Vital::Sandbox::API::bind(vm, "adjustment", "is_enabled", [](auto* vm) -> int {
                vm -> push_bool(Vital::Godot::Core::get_environment() -> is_adjustment_enabled());
                return 1;
            });
        
            Vital::Sandbox::API::bind(vm, "adjustment", "set_brightness", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                Vital::Godot::Core::get_environment() -> set_adjustment_brightness(value);
                vm -> push_bool(true);
                return 1;
            });
        
            Vital::Sandbox::API::bind(vm, "adjustment", "get_brightness", [](auto* vm) -> int {
                vm -> push_number(Vital::Godot::Core::get_environment() -> get_adjustment_brightness());
                return 1;
            });
        
            Vital::Sandbox::API::bind(vm, "adjustment", "set_contrast", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                Vital::Godot::Core::get_environment() -> set_adjustment_contrast(value);
                vm -> push_bool(true);
                return 1;
            });
        
            Vital::Sandbox::API::bind(vm, "adjustment", "get_contrast", [](auto* vm) -> int {
                vm -> push_number(Vital::Godot::Core::get_environment() -> get_adjustment_contrast());
                return 1;
            });
        
            Vital::Sandbox::API::bind(vm, "adjustment", "set_saturation", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                Vital::Godot::Core::get_environment() -> set_adjustment_saturation(value);
                vm -> push_bool(true);
                return 1;
            });
        
            Vital::Sandbox::API::bind(vm, "adjustment", "get_saturation", [](auto* vm) -> int {
                vm -> push_number(Vital::Godot::Core::get_environment() -> get_adjustment_saturation());
                return 1;
            });
            #endif
        }
    };
}