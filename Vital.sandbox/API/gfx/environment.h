/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: environment.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Environment APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>


//////////////////////////////
// Vital: API: Environment //
//////////////////////////////

namespace Vital::Sandbox::API {
    struct Environment : vm_module {
        inline static const std::vector<std::string> base_scope = {"gfx", "env"};
        using base_class = Vital::Engine::Core;

        inline static const std::vector<std::pair<std::string, int>> background_mode_registry = {
            { "CLEAR_COLOR", godot::Environment::BG_CLEAR_COLOR },
            { "COLOR", godot::Environment::BG_COLOR },
            { "SKY", godot::Environment::BG_SKY }
        };

        inline static const std::vector<std::pair<std::string, int>> ambient_source_registry = {
            { "BG", godot::Environment::AMBIENT_SOURCE_BG },
            { "DISABLED", godot::Environment::AMBIENT_SOURCE_DISABLED },
            { "COLOR", godot::Environment::AMBIENT_SOURCE_COLOR },
            { "SKY", godot::Environment::AMBIENT_SOURCE_SKY }
        };

        inline static const std::vector<std::pair<std::string, int>> reflection_source_registry = {
            { "BG", godot::Environment::REFLECTION_SOURCE_BG },
            { "DISABLED", godot::Environment::REFLECTION_SOURCE_DISABLED },
            { "SKY", godot::Environment::REFLECTION_SOURCE_SKY }
        };

        inline static const std::vector<std::pair<std::string, int>> tonemapper_mode_registry = {
            { "LINEAR", godot::Environment::TONE_MAPPER_LINEAR },
            { "REINHARDT", godot::Environment::TONE_MAPPER_REINHARDT },
            { "FILMIC", godot::Environment::TONE_MAPPER_FILMIC },
            { "ACES", godot::Environment::TONE_MAPPER_ACES }
        };

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "get_background_mode", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_environment() -> get_background()));
                return 1;
            });

            API::bind(vm, base_scope, "get_background_color", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_bg_color());
                return 1;
            });

            API::bind(vm, base_scope, "get_background_energy", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_bg_energy_multiplier());
                return 1;
            });

            API::bind(vm, base_scope, "get_background_intensity", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_bg_intensity());
                return 1;
            });

            API::bind(vm, base_scope, "get_ambient_source", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_environment() -> get_ambient_source()));
                return 1;
            });

            API::bind(vm, base_scope, "get_ambient_color", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ambient_light_color());
                return 1;
            });

            API::bind(vm, base_scope, "get_ambient_energy", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ambient_light_energy());
                return 1;
            });

            API::bind(vm, base_scope, "get_ambient_sky_contribution", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ambient_light_sky_contribution());
                return 1;
            });

            API::bind(vm, base_scope, "get_reflection_source", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_environment() -> get_reflection_source()));
                return 1;
            });
    
            API::bind(vm, base_scope, "get_tonemapper_mode", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_environment() -> get_tonemapper()));
                return 1;
            });

            API::bind(vm, base_scope, "get_tonemap_exposure", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_tonemap_exposure());
                return 1;
            });

            API::bind(vm, base_scope, "get_tonemap_white", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_tonemap_white());
                return 1;
            });
            
            API::bind(vm, base_scope, "set_background_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require_enum(1, background_mode_registry);

                auto mode = static_cast<godot::Environment::BGMode>(vm -> get_int(1));
                base_class::get_environment() -> set_background(mode);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_background_color", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto value = vm -> get_color(1);
                base_class::get_environment() -> set_bg_color(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_background_energy", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_bg_energy_multiplier(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_background_intensity", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_bg_intensity(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_ambient_source", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(source)")
                    .require_enum(1, ambient_source_registry);

                auto source = static_cast<godot::Environment::AmbientSource>(vm -> get_int(1));
                base_class::get_environment() -> set_ambient_source(source);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_ambient_color", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto value = vm -> get_color(1);
                base_class::get_environment() -> set_ambient_light_color(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_ambient_energy", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ambient_light_energy(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_ambient_sky_contribution", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ambient_light_sky_contribution(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_reflection_source", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(source)")
                    .require_enum(1, reflection_source_registry);

                auto source = static_cast<godot::Environment::ReflectionSource>(vm -> get_int(1));
                base_class::get_environment() -> set_reflection_source(source);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_tonemapper_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require_enum(1, tonemapper_mode_registry);

                auto mode = static_cast<godot::Environment::ToneMapper>(vm -> get_int(1));
                base_class::get_environment() -> set_tonemapper(mode);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_tonemap_exposure", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_tonemap_exposure(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_tonemap_white", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_tonemap_white(value);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "background_mode", background_mode_registry);
            vm -> scope_set_enum(base_scope, "ambient_source", ambient_source_registry);
            vm -> scope_set_enum(base_scope, "reflection_source", reflection_source_registry);
            vm -> scope_set_enum(base_scope, "tonemapper_mode", tonemapper_mode_registry);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Environment : vm_module {};
}
#endif