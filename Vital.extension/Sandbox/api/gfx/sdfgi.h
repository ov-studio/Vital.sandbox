/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: gfx: sdfgi.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SDFGI APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/index.h>


/////////////////////////////////
// Vital: Sandbox: API: SDFGI //
/////////////////////////////////

namespace Vital::Sandbox::API {
    struct SDFGI : vm_module {
        inline static const std::string base_name = "gfx";
        using base_class = Vital::Engine::Core;

        static void bind(Machine* vm) {
            #if defined(Vital_SDK_Client)
            API::bind(vm, {base_name, "sdfgi"}, "is_enabled", [](auto vm) -> int {
                vm -> push_bool(base_class::get_environment() -> is_sdfgi_enabled());
                return 1;
            });

            API::bind(vm, {base_name, "sdfgi"}, "set_enabled", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_bool(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_sdfgi_enabled(state);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "set_cascades", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_int(1);
                base_class::get_environment() -> set_sdfgi_cascades(value);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "get_cascades", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_sdfgi_cascades());
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "set_min_cell_size", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sdfgi_min_cell_size(value);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "get_min_cell_size", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_sdfgi_min_cell_size());
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "set_max_distance", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sdfgi_max_distance(value);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "get_max_distance", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_sdfgi_max_distance());
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "set_y_scale", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_int(1);
                if ((value < godot::Environment::SDFGI_Y_SCALE_50_PERCENT) || (value > godot::Environment::SDFGI_Y_SCALE_100_PERCENT)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                base_class::get_environment() -> set_sdfgi_y_scale(static_cast<godot::Environment::SDFGIYScale>(value));
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "get_y_scale", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_sdfgi_y_scale());
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "is_using_occlusion", [](auto vm) -> int {
                vm -> push_bool(base_class::get_environment() -> is_sdfgi_using_occlusion());
                return 1;
            });

            API::bind(vm, {base_name, "sdfgi"}, "set_use_occlusion", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_bool(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_sdfgi_use_occlusion(state);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "set_bounce_feedback", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sdfgi_bounce_feedback(value);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "get_bounce_feedback", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_sdfgi_bounce_feedback());
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "is_reading_sky_light", [](auto vm) -> int {
                vm -> push_bool(base_class::get_environment() -> is_sdfgi_reading_sky_light());
                return 1;
            });

            API::bind(vm, {base_name, "sdfgi"}, "set_read_sky_light", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_bool(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_sdfgi_read_sky_light(state);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "set_energy", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sdfgi_energy(value);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "get_energy", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_sdfgi_energy());
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "set_normal_bias", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sdfgi_normal_bias(value);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "get_normal_bias", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_sdfgi_normal_bias());
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "set_probe_bias", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sdfgi_probe_bias(value);
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "sdfgi"}, "get_probe_bias", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_sdfgi_probe_bias());
                return 1;
            });
            #endif
        }
    };
}