/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: private: volumetric_fog.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Volumetric Fog APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/api/public/volumetric_fog.h>


/////////////////////////////////////////////////////
// Vital: Godot: Sandbox: Lua: API: VolumetricFog //
/////////////////////////////////////////////////////

void Vital::Godot::Sandbox::Lua::API::VolumetricFog::bind(void* instance) {
    auto vm = Vital::Sandbox::Lua::create::to_vm(instance);

    #if defined(Vital_SDK_Client)
    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_bool(1))) throw Vital::Error::fetch("invalid-arguments");
            auto state = vm -> get_bool(1);
            Vital::Godot::Core::get_environment() -> set_volumetric_fog_enabled(state);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "is_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_bool(Vital::Godot::Core::get_environment() -> is_volumetric_fog_enabled());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_emission", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if (vm -> is_string(1)) {
                auto value = to_godot_string(vm -> get_string(1));
                if (godot::Color::html_is_valid(value)) throw Vital::Error::fetch("invalid-arguments");
                Vital::Godot::Core::get_environment() -> set_volumetric_fog_emission(godot::Color::html(value));
            }
            else {
                if (vm -> get_arg_count() < 4) throw Vital::Error::fetch("invalid-arguments");
                for (int i = 1; i <= 4; i++) {
                    if (!vm -> is_number(i)) throw Vital::Error::fetch("invalid-arguments");
                }
                Vital::Godot::Core::get_environment() -> set_volumetric_fog_emission(godot::Color(
                    vm -> get_float(1), 
                    vm -> get_float(2), 
                    vm -> get_float(3), 
                    vm -> get_float(4)
                ));
            }
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "get_emission", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            auto value = Vital::Godot::Core::get_environment() -> get_volumetric_fog_emission();
            vm -> set_number(value.r);
            vm -> set_number(value.g);
            vm -> set_number(value.b);
            vm -> set_number(value.a);
            return 4;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_albedo", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if (vm -> is_string(1)) {
                auto value = to_godot_string(vm -> get_string(1));
                if (godot::Color::html_is_valid(value)) throw Vital::Error::fetch("invalid-arguments");
                Vital::Godot::Core::get_environment() -> set_volumetric_fog_albedo(godot::Color::html(value));
            }
            else {
                if (vm -> get_arg_count() < 4) throw Vital::Error::fetch("invalid-arguments");
                for (int i = 1; i <= 4; i++) {
                    if (!vm -> is_number(i)) throw Vital::Error::fetch("invalid-arguments");
                }
                Vital::Godot::Core::get_environment() -> set_volumetric_fog_albedo(godot::Color(
                    vm -> get_float(1), 
                    vm -> get_float(2), 
                    vm -> get_float(3), 
                    vm -> get_float(4)
                ));
            }
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "get_albedo", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            auto value = Vital::Godot::Core::get_environment() -> get_volumetric_fog_albedo();
            vm -> set_number(value.r);
            vm -> set_number(value.g);
            vm -> set_number(value.b);
            vm -> set_number(value.a);
            return 4;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_density", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_volumetric_fog_density(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "get_density", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_volumetric_fog_density());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_emission_energy", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_volumetric_fog_emission_energy(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "get_emission_energy", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_volumetric_fog_emission_energy());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_anisotropy", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_volumetric_fog_anisotropy(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "get_anisotropy", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_volumetric_fog_anisotropy());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_length", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_volumetric_fog_length(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "get_length", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_volumetric_fog_length());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_detail_spread", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_volumetric_fog_detail_spread(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "get_detail_spread", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_volumetric_fog_detail_spread());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_gi_inject", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_volumetric_fog_gi_inject(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "get_gi_inject", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_volumetric_fog_gi_inject());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_ambient_inject", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_volumetric_fog_ambient_inject(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "get_ambient_inject", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_volumetric_fog_ambient_inject());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_sky_affect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_volumetric_fog_sky_affect(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "get_sky_affect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_volumetric_fog_sky_affect());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_temporal_reprojection_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_bool(1))) throw Vital::Error::fetch("invalid-arguments");
            auto state = vm -> get_bool(1);
            Vital::Godot::Core::get_environment() -> set_volumetric_fog_temporal_reprojection_enabled(state);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "is_temporal_reprojection_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_bool(Vital::Godot::Core::get_environment() -> is_volumetric_fog_temporal_reprojection_enabled());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "set_temporal_reprojection_amount", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_volumetric_fog_temporal_reprojection_amount(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "get_temporal_reprojection_amount", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_volumetric_fog_temporal_reprojection_amount());
            return 1;
        });
    });
    #endif
}