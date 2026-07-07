/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: sky_physical.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sky (Physical) APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/API/gfx/sky.h>


///////////////////////////////
// Vital: API: Sky_Physical //
///////////////////////////////

namespace Vital::Sandbox::API {
    struct Sky_Physical : vm_module {
        inline static const std::vector<std::string> base_scope = {"gfx", "sky", "physical"};
        inline static const std::string night_sky_reference = fmt::format("{}:night_sky", vm_module::scope_id(base_scope));

        static void init(Machine* vm) {
            static bool initialized = false;
            if (initialized) return;
            initialized = true;

            Tool::Event::bind("environment:free", [vm](Tool::Stack args) {
                vm -> del_reference("sandbox", night_sky_reference);
            });
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "get_rayleigh_coefficient", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::PhysicalSkyMaterial>() -> get_rayleigh_coefficient());
                return 1;
            });

            API::bind(vm, base_scope, "get_rayleigh_color", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::PhysicalSkyMaterial>() -> get_rayleigh_color());
                return 1;
            });

            API::bind(vm, base_scope, "get_mie_coefficient", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::PhysicalSkyMaterial>() -> get_mie_coefficient());
                return 1;
            });

            API::bind(vm, base_scope, "get_mie_eccentricity", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::PhysicalSkyMaterial>() -> get_mie_eccentricity());
                return 1;
            });

            API::bind(vm, base_scope, "get_mie_color", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::PhysicalSkyMaterial>() -> get_mie_color());
                return 1;
            });

            API::bind(vm, base_scope, "get_turbidity", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::PhysicalSkyMaterial>() -> get_turbidity());
                return 1;
            });

            API::bind(vm, base_scope, "get_sun_disk_scale", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::PhysicalSkyMaterial>() -> get_sun_disk_scale());
                return 1;
            });

            API::bind(vm, base_scope, "get_ground_color", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::PhysicalSkyMaterial>() -> get_ground_color());
                return 1;
            });

            API::bind(vm, base_scope, "get_energy_multiplier", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::PhysicalSkyMaterial>() -> get_energy_multiplier());
                return 1;
            });

            API::bind(vm, base_scope, "get_use_debanding", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::ensure_material<godot::PhysicalSkyMaterial>() -> get_use_debanding());
                return 1;
            });

            API::bind(vm, base_scope, "get_night_sky", [](auto vm, auto& id) -> int {
                if (vm -> is_reference("sandbox", night_sky_reference)) vm -> get_reference("sandbox", night_sky_reference, true);
                else vm -> push_value(false);
                return 1;
            });

            API::bind(vm, base_scope, "set_rayleigh_coefficient", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::PhysicalSkyMaterial>() -> set_rayleigh_coefficient(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_rayleigh_color", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto value = vm -> get_color(1);
                Sky::ensure_material<godot::PhysicalSkyMaterial>() -> set_rayleigh_color(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_mie_coefficient", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::PhysicalSkyMaterial>() -> set_mie_coefficient(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_mie_eccentricity", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::PhysicalSkyMaterial>() -> set_mie_eccentricity(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_mie_color", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto value = vm -> get_color(1);
                Sky::ensure_material<godot::PhysicalSkyMaterial>() -> set_mie_color(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_turbidity", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::PhysicalSkyMaterial>() -> set_turbidity(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_sun_disk_scale", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::PhysicalSkyMaterial>() -> set_sun_disk_scale(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_ground_color", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto value = vm -> get_color(1);
                Sky::ensure_material<godot::PhysicalSkyMaterial>() -> set_ground_color(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_energy_multiplier", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::ensure_material<godot::PhysicalSkyMaterial>() -> set_energy_multiplier(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_use_debanding", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                Sky::ensure_material<godot::PhysicalSkyMaterial>() -> set_use_debanding(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_night_sky", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path)")
                    .require(1, &Machine::is_string);

                auto path = vm -> get_string(1);
                auto ref = path;
                auto base = API::File::assert_file(vm, path);
                auto texture = Vital::Engine::Texture::get_from_reference(ref);
                if (!texture) texture = Vital::Engine::Texture::create_texture_2d(base, path, ref);
                Sky::ensure_material<godot::PhysicalSkyMaterial>() -> set_night_sky(texture -> get_texture());
                vm -> push_value(path);
                vm -> set_reference("sandbox", night_sky_reference, -1);
                vm -> pop(1);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "reset_night_sky", [](auto vm, auto& id) -> int {
                Sky::ensure_material<godot::PhysicalSkyMaterial>() -> set_night_sky(godot::Ref<godot::Texture>());
                vm -> del_reference("sandbox", night_sky_reference);
                vm -> push_value(true);
                return 1;
            });
        }
    };
}

#else
namespace Vital::Sandbox::API {
    struct Sky_Physical : vm_module {};
}
#endif