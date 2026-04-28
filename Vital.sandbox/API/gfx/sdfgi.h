/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: sdfgi.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SDFGI APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>


////////////////////////
// Vital: API: SDFGI //
////////////////////////

namespace Vital::Sandbox::API {
    struct SDFGI : vm_module {
        inline static const std::string base_name = "sdfgi";
        using base_class = Vital::Engine::Core;

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "is_enabled", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> is_sdfgi_enabled());
                return 1;
            });

            API::bind(vm, {base_name}, "set_enabled", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_sdfgi_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "set_cascades", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_int(1);
                base_class::get_environment() -> set_sdfgi_cascades(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_cascades", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sdfgi_cascades());
                return 1;
            });

            API::bind(vm, {base_name}, "set_min_cell_size", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sdfgi_min_cell_size(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_min_cell_size", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sdfgi_min_cell_size());
                return 1;
            });

            API::bind(vm, {base_name}, "set_max_distance", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sdfgi_max_distance(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_max_distance", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sdfgi_max_distance());
                return 1;
            });

            API::bind(vm, {base_name}, "set_y_scale", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number)
                    .validate(1, [](auto vm, int index) {
                        auto value = vm -> get_int(index);
                        return (value >= godot::Environment::SDFGI_Y_SCALE_50_PERCENT) && (value <= godot::Environment::SDFGI_Y_SCALE_100_PERCENT);
                    });

                auto value = vm -> get_int(1);
                base_class::get_environment() -> set_sdfgi_y_scale(static_cast<godot::Environment::SDFGIYScale>(value));
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_y_scale", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sdfgi_y_scale());
                return 1;
            });

            API::bind(vm, {base_name}, "is_using_occlusion", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> is_sdfgi_using_occlusion());
                return 1;
            });

            API::bind(vm, {base_name}, "set_use_occlusion", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_sdfgi_use_occlusion(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "set_bounce_feedback", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sdfgi_bounce_feedback(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_bounce_feedback", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sdfgi_bounce_feedback());
                return 1;
            });

            API::bind(vm, {base_name}, "is_reading_sky_light", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> is_sdfgi_reading_sky_light());
                return 1;
            });

            API::bind(vm, {base_name}, "set_read_sky_light", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_sdfgi_read_sky_light(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "set_energy", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sdfgi_energy(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_energy", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sdfgi_energy());
                return 1;
            });

            API::bind(vm, {base_name}, "set_normal_bias", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sdfgi_normal_bias(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_normal_bias", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sdfgi_normal_bias());
                return 1;
            });

            API::bind(vm, {base_name}, "set_probe_bias", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sdfgi_probe_bias(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_probe_bias", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sdfgi_probe_bias());
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct SDFGI : vm_module {};
}
#endif