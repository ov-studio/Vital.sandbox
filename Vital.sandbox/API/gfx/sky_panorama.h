/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: sky_panorama.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sky (Panorama) APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/API/gfx/sky.h>


///////////////////////////////
// Vital: API: Sky_Panorama //
///////////////////////////////

namespace Vital::Sandbox::API {
    struct Sky_Panorama : vm_module {
        inline static const std::vector<std::string> base_scope = {"gfx", "sky", "panorama"};

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "get_texture", [](auto vm, auto& id) -> int {
                auto mat = Sky::Sky::ensure_material<godot::PanoramaSkyMaterial>();
                auto texture = mat -> get_panorama();
                vm -> push_value(texture.is_valid());
                return 1;
            });

            API::bind(vm, base_scope, "set_texture", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path)")
                    .require(1, &Machine::is_string);

                auto path = vm -> get_string(1);
                auto ref = path;
                auto base = API::File::assert_file(vm, path);
                auto texture = Vital::Engine::Texture::get_from_reference(ref);
                if (!texture) texture = Vital::Engine::Texture::create_texture_2d(base, path, ref);
                Sky::Sky::ensure_material<godot::PanoramaSkyMaterial>() -> set_panorama(texture -> get_texture());
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "is_filtering_enabled", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::Sky::ensure_material<godot::PanoramaSkyMaterial>() -> is_filtering_enabled());
                return 1;
            });

            API::bind(vm, base_scope, "set_filtering_enabled", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                Sky::Sky::ensure_material<godot::PanoramaSkyMaterial>() -> set_filtering_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_energy_multiplier", [](auto vm, auto& id) -> int {
                vm -> push_value(Sky::Sky::ensure_material<godot::PanoramaSkyMaterial>() -> get_energy_multiplier());
                return 1;
            });

            API::bind(vm, base_scope, "set_energy_multiplier", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                Sky::Sky::ensure_material<godot::PanoramaSkyMaterial>() -> set_energy_multiplier(value);
                vm -> push_value(true);
                return 1;
            });
        }
    };
}

#else
namespace Vital::Sandbox::API {
    struct Sky_Panorama : vm_module {};
}
#endif
