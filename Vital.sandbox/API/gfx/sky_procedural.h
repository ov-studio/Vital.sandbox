/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: sky_procedural.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sky (Procedural) APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/API/gfx/sky.h>


/////////////////////////////////
// Vital: API: Sky_Procedural //
/////////////////////////////////

namespace Vital::Sandbox::API {
    struct Sky_Procedural : vm_module {
        inline static const std::vector<std::string> base_scope = {"gfx", "sky", "procedural"};
        inline static const std::string sky_cover_reference = fmt::format("{}:sky_cover", vm_module::scope_id(base_scope));

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "get_sky_top_color", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_sky_top_color());
                return 1;
            });

            API::bind(vm, base_scope, "set_sky_top_color", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto value = vm -> get_color(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_sky_top_color(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_sky_horizon_color", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_sky_horizon_color());
                return 1;
            });

            API::bind(vm, base_scope, "set_sky_horizon_color", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto value = vm -> get_color(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_sky_horizon_color(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_sky_curve", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_sky_curve());
                return 1;
            });

            API::bind(vm, base_scope, "set_sky_curve", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_sky_curve(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_sky_energy_multiplier", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_sky_energy_multiplier());
                return 1;
            });

            API::bind(vm, base_scope, "set_sky_energy_multiplier", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_sky_energy_multiplier(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_sky_cover", [](auto vm, auto& id) -> int {
                if (vm -> is_reference("sandbox", sky_cover_reference)) vm -> get_reference("sandbox", sky_cover_reference, true);
                else vm -> push_value(false);
                return 1;
            });

            API::bind(vm, base_scope, "set_sky_cover", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path)")
                    .require(1, &Machine::is_string);

                auto path = vm -> get_string(1);
                auto ref = path;
                auto base = API::File::assert_file(vm, path);
                auto texture = Vital::Engine::Texture::get_from_reference(ref);
                if (!texture) texture = Vital::Engine::Texture::create_texture_2d(base, path, ref);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_sky_cover(texture -> get_texture());
                vm -> push_value(path);
                vm -> set_reference("sandbox", sky_cover_reference, -1);
                vm -> pop(1);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_sky_cover_modulate", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_sky_cover_modulate());
                return 1;
            });

            API::bind(vm, base_scope, "set_sky_cover_modulate", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto value = vm -> get_color(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_sky_cover_modulate(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_ground_bottom_color", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_ground_bottom_color());
                return 1;
            });

            API::bind(vm, base_scope, "set_ground_bottom_color", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto value = vm -> get_color(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_ground_bottom_color(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_ground_horizon_color", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_ground_horizon_color());
                return 1;
            });

            API::bind(vm, base_scope, "set_ground_horizon_color", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto value = vm -> get_color(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_ground_horizon_color(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_ground_curve", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_ground_curve());
                return 1;
            });

            API::bind(vm, base_scope, "set_ground_curve", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_ground_curve(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_ground_energy_multiplier", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_ground_energy_multiplier());
                return 1;
            });

            API::bind(vm, base_scope, "set_ground_energy_multiplier", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_ground_energy_multiplier(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_sun_angle_max", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_sun_angle_max());
                return 1;
            });

            API::bind(vm, base_scope, "set_sun_angle_max", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_sun_angle_max(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_sun_curve", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_sun_curve());
                return 1;
            });

            API::bind(vm, base_scope, "set_sun_curve", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_sun_curve(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_use_debanding", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_use_debanding());
                return 1;
            });

            API::bind(vm, base_scope, "set_use_debanding", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_use_debanding(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_energy_multiplier", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::ProceduralSkyMaterial>() -> get_energy_multiplier());
                return 1;
            });

            API::bind(vm, base_scope, "set_energy_multiplier", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::ProceduralSkyMaterial>() -> set_energy_multiplier(value);
                vm -> push_value(true);
                return 1;
            });
        }
    };
}

#else
namespace Vital::Sandbox::API {
    struct Sky_Procedural : vm_module {};
}
#endif
