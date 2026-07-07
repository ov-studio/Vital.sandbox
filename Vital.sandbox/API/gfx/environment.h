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
        inline static const std::vector<std::string> base_scope = {"gfx", "environment"};
        using base_class = Vital::Engine::Core;

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "get_background_mode", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_environment() -> get_background()));
                return 1;
            });

            API::bind(vm, base_scope, "set_background_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require(1, &Machine::is_number)
                    .validate_enum(1, godot::Environment::BG_CLEAR_COLOR, godot::Environment::BG_CAMERA_FEED);

                auto mode = static_cast<godot::Environment::BGMode>(vm -> get_int(1));
                base_class::get_environment() -> set_background(mode);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, base_scope, "get_background_color", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_bg_color());
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

            API::bind(vm, base_scope, "get_background_energy_multiplier", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_bg_energy_multiplier());
                return 1;
            });

            API::bind(vm, base_scope, "set_background_energy_multiplier", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_bg_energy_multiplier(value);
                vm -> push_value(true);
                return 1;
            });
    
            API::bind(vm, base_scope, "get_background_intensity", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_bg_intensity());
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

            API::bind(vm, base_scope, "get_sky_fov", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sky_custom_fov());
                return 1;
            });

            API::bind(vm, base_scope, "set_sky_fov", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sky_custom_fov(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_sky_rotation", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sky_rotation());
                return 1;
            });

            API::bind(vm, base_scope, "set_sky_rotation", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(rotation)")
                    .require(1, &Machine::is_vector3);

                auto value = vm -> get_vector3(1);
                base_class::get_environment() -> set_sky_rotation(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, base_scope, "get_ambient_color", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ambient_light_color());
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

            API::bind(vm, base_scope, "get_ambient_source", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_environment() -> get_ambient_source()));
                return 1;
            });

            API::bind(vm, base_scope, "set_ambient_source", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(source)")
                    .require(1, &Machine::is_number)
                    .validate_enum(1, godot::Environment::AMBIENT_SOURCE_BG, godot::Environment::AMBIENT_SOURCE_SKY);

                auto source = static_cast<godot::Environment::AmbientSource>(vm -> get_int(1));
                base_class::get_environment() -> set_ambient_source(source);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_ambient_energy", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ambient_light_energy());
                return 1;
            });

            API::bind(vm, base_scope, "set_ambient_energy", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(energy)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ambient_light_energy(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_ambient_sky_contribution", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ambient_light_sky_contribution());
                return 1;
            });

            API::bind(vm, base_scope, "set_ambient_sky_contribution", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(ratio)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ambient_light_sky_contribution(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_reflection_source", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_environment() -> get_reflection_source()));
                return 1;
            });

            API::bind(vm, base_scope, "set_reflection_source", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(source)")
                    .require(1, &Machine::is_number)
                    .validate_enum(1, godot::Environment::REFLECTION_SOURCE_BG, godot::Environment::REFLECTION_SOURCE_SKY);

                auto source = static_cast<godot::Environment::ReflectionSource>(vm -> get_int(1));
                base_class::get_environment() -> set_reflection_source(source);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, base_scope, "get_tonemapper_mode", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_environment() -> get_tonemapper()));
                return 1;
            });

            API::bind(vm, base_scope, "set_tonemapper_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require(1, &Machine::is_number)
                    .validate_enum(1, godot::Environment::TONE_MAPPER_LINEAR, godot::Environment::TONE_MAPPER_ACES);

                auto mode = static_cast<godot::Environment::ToneMapper>(vm -> get_int(1));
                base_class::get_environment() -> set_tonemapper(mode);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_tonemap_exposure", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_tonemap_exposure());
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

            API::bind(vm, base_scope, "get_tonemap_white", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_tonemap_white());
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

            // Ambient // TODO: REMOVE
            API::bind(vm, base_scope, "get_canvas_max_layer", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_canvas_max_layer());
                return 1;
            });

            API::bind(vm, base_scope, "set_canvas_max_layer", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(layer)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_int(1);
                base_class::get_environment() -> set_canvas_max_layer(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_camera_feed_id", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_camera_feed_id());
                return 1;
            });

            API::bind(vm, base_scope, "set_camera_feed_id", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(id)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_int(1);
                base_class::get_environment() -> set_camera_feed_id(value);
                vm -> push_value(true);
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Adjustment : vm_module {};
}
#endif