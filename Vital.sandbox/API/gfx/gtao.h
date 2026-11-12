/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: gtao.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 10/11/2026
     Desc: GTAO APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/gtao.h>


///////////////////////
// Vital: API: GTAO //
///////////////////////

// TODO: Improve

namespace Vital::Sandbox::API {
    struct GTAO : vm_module {
        inline static const std::vector<std::string> base_scope = {"gfx", "gtao"};
        using base_class = Vital::Engine::GTAO;

        inline static const std::vector<std::pair<std::string, int>> quality_registry = {
            { "LOW",    0 },
            { "MEDIUM", 1 },
            { "HIGH",   2 },
            { "ULTRA",  3 }
        };

        inline static const std::vector<std::pair<std::string, int>> denoise_mode_registry = {
            { "OFF",    0 },
            { "SHARP",  1 },
            { "MEDIUM", 2 },
            { "SOFT",   3 }
        };

        static void init(Machine* vm) {
            static Tool::Event::Handle reset_binding;
            reset_binding.bind("environment:free", [](Tool::Stack) {
                base_class::reset();
            });
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "is_enabled", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::is_enabled());
                return 1;
            });

            API::bind(vm, base_scope, "is_half_res", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().half_res != 0);
                return 1;
            });

            API::bind(vm, base_scope, "get_quality", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().quality);
                return 1;
            });

            API::bind(vm, base_scope, "get_denoise", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().denoise);
                return 1;
            });

            API::bind(vm, base_scope, "get_radius", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().radius);
                return 1;
            });

            API::bind(vm, base_scope, "get_intensity", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().intensity);
                return 1;
            });

            API::bind(vm, base_scope, "get_radius_multiplier", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().radius_multiplier);
                return 1;
            });

            API::bind(vm, base_scope, "get_falloff", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().falloff_range);
                return 1;
            });

            API::bind(vm, base_scope, "get_distribution", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().sample_distribution_power);
                return 1;
            });

            API::bind(vm, base_scope, "get_thin_occluder", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().thin_occluder_compensation);
                return 1;
            });

            API::bind(vm, base_scope, "get_power", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().final_value_power);
                return 1;
            });

            API::bind(vm, base_scope, "get_mip_offset", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().depth_mip_sampling_offset);
                return 1;
            });

            API::bind(vm, base_scope, "get_blur", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().blur);
                return 1;
            });

            API::bind(vm, base_scope, "get_smooth", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_settings().smooth_radius);
                return 1;
            });

            API::bind(vm, base_scope, "set_enabled", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::set_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_half_res", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::update_settings([&](auto& settings) { settings.half_res = state ? 1 : 0; });
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_quality", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(quality)")
                    .require_enum(1, quality_registry);

                auto value = vm -> get_int(1);
                base_class::update_settings([&](auto& settings) { settings.quality = value; });
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_denoise", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require_enum(1, denoise_mode_registry);

                auto value = vm -> get_int(1);
                base_class::update_settings([&](auto& settings) { settings.denoise = value; });
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_radius", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::update_settings([&](auto& settings) { settings.radius = value; });
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_intensity", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::update_settings([&](auto& settings) { settings.intensity = value; });
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_radius_multiplier", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::update_settings([&](auto& settings) { settings.radius_multiplier = value; });
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_falloff", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::update_settings([&](auto& settings) { settings.falloff_range = value; });
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_distribution", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::update_settings([&](auto& settings) { settings.sample_distribution_power = value; });
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_thin_occluder", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::update_settings([&](auto& settings) { settings.thin_occluder_compensation = value; });
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_power", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::update_settings([&](auto& settings) { settings.final_value_power = value; });
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_mip_offset", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::update_settings([&](auto& settings) { settings.depth_mip_sampling_offset = value; });
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_blur", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::update_settings([&](auto& settings) { settings.blur = value; });
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_smooth", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_int(1);
                base_class::update_settings([&](auto& settings) { settings.smooth_radius = value; });
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "quality", quality_registry);
            vm -> scope_set_enum(base_scope, "denoise_mode", denoise_mode_registry);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct GTAO : vm_module {};
}
#endif