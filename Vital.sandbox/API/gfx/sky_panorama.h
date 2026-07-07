/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: sky_panorama.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sky Panorama APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once


////////////////////////////////
// Vital: API: Sky: Panorama //
////////////////////////////////

namespace Vital::Sandbox::API {
    inline void Sky::bind_panorama(Machine* vm) {
        API::bind(vm, panorama_scope, "get_texture", [](auto vm, auto& id) -> int {
            auto mat = ensure_material<godot::PanoramaSkyMaterial>();
            auto texture = mat -> get_panorama();
            vm -> push_value(texture.is_valid());
            return 1;
        });

        API::bind(vm, panorama_scope, "set_texture", [](auto vm, auto& id) -> int {
            vm_args(vm, id, "(path)")
                .require(1, &Machine::is_string);

            auto path = vm -> get_string(1);
            auto ref = path;
            auto base = API::File::assert_file(vm, path);
            auto texture = Vital::Engine::Texture::get_from_reference(ref);
            if (!texture) texture = Vital::Engine::Texture::create_texture_2d(base, path, ref);
            ensure_material<godot::PanoramaSkyMaterial>() -> set_panorama(texture -> get_texture());
            vm -> push_value(true);
            return 1;
        });

        API::bind(vm, panorama_scope, "is_filtering_enabled", [](auto vm, auto& id) -> int {
            vm -> push_value(ensure_material<godot::PanoramaSkyMaterial>() -> is_filtering_enabled());
            return 1;
        });

        API::bind(vm, panorama_scope, "set_filtering_enabled", [](auto vm, auto& id) -> int {
            vm_args(vm, id, "(state)")
                .require(1, &Machine::is_bool);

            auto state = vm -> get_bool(1);
            ensure_material<godot::PanoramaSkyMaterial>() -> set_filtering_enabled(state);
            vm -> push_value(true);
            return 1;
        });

        API::bind(vm, panorama_scope, "get_energy_multiplier", [](auto vm, auto& id) -> int {
            vm -> push_value(ensure_material<godot::PanoramaSkyMaterial>() -> get_energy_multiplier());
            return 1;
        });

        API::bind(vm, panorama_scope, "set_energy_multiplier", [](auto vm, auto& id) -> int {
            vm_args(vm, id, "(value)")
                .require(1, &Machine::is_number);

            auto value = vm -> get_float(1);
            ensure_material<godot::PanoramaSkyMaterial>() -> set_energy_multiplier(value);
            vm -> push_value(true);
            return 1;
        });
    }
}