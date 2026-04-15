/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: fog.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Fog APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>


//////////////////////
// Vital: API: Fog //
//////////////////////

namespace Vital::Sandbox::API {
    struct Fog : vm_module {
        inline static const std::string base_name = "gfx";
        using base_class = Vital::Engine::Core;

        static void bind(Machine* vm) {
            API::bind(vm, {base_name, "fog"}, "is_enabled", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> is_fog_enabled());
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_enabled", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_fog_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number)
                    .validate(1, [](auto vm, int index) {
                        auto value = vm -> get_int(index);
                        return (value >= godot::Environment::FOG_MODE_EXPONENTIAL) && (value <= godot::Environment::FOG_MODE_DEPTH);
                    });

                auto value = vm -> get_int(1);
                base_class::get_environment() -> set_fog_mode(static_cast<godot::Environment::FogMode>(value));
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "get_mode", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_fog_mode());
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_light_color", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto color = vm -> get_color(1);
                base_class::get_environment() -> set_fog_light_color(color);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "get_light_color", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_fog_light_color());
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_light_energy", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_fog_light_energy(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "get_light_energy", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_fog_light_energy());
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_sun_scatter", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_fog_sun_scatter(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "get_sun_scatter", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_fog_sun_scatter());
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_density", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_fog_density(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "get_density", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_fog_density());
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_height", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_fog_height(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "get_height", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_fog_height());
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_height_density", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_fog_height_density(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "get_height_density", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_fog_height_density());
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_aerial_perspective", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_fog_aerial_perspective(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "get_aerial_perspective", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_fog_aerial_perspective());
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_sky_affect", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_fog_sky_affect(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "get_sky_affect", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_fog_sky_affect());
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_depth_curve", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_fog_depth_curve(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "get_depth_curve", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_fog_depth_curve());
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_depth_begin", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_fog_depth_begin(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "get_depth_begin", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_fog_depth_begin());
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "set_depth_end", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_fog_depth_end(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "fog"}, "get_depth_end", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_fog_depth_end());
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Fog : vm_module {};
}
#endif