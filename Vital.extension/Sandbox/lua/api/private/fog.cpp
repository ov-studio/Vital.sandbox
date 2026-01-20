/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: private: fog.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Fog APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/api/public/fog.h>


////////////////////////////////////////
// Vital: Sandbox: API: Fog //
////////////////////////////////////////

void Vital::Sandbox::API::Fog::bind(void* machine) {
    auto vm = Machine::to_machine(machine);

    #if defined(Vital_SDK_Client)
    Vital::Sandbox::API::bind(vm, "fog", "set_enabled", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_bool(1))) throw Vital::Error::fetch("invalid-arguments");
            auto state = vm -> get_bool(1);
            Vital::Godot::Core::get_environment() -> set_fog_enabled(state);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "is_enabled", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            vm -> push_bool(Vital::Godot::Core::get_environment() -> is_fog_enabled());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "set_mode", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_int(1);
            if ((value < godot::Environment::FOG_MODE_EXPONENTIAL) || (value > godot::Environment::FOG_MODE_DEPTH)) throw Vital::Error::fetch("invalid-arguments");
            Vital::Godot::Core::get_environment() -> set_fog_mode(static_cast<godot::Environment::FogMode>(value));
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "get_mode", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_fog_mode());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "set_light_color", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if (vm -> is_string(1)) {
                auto value = to_godot_string(vm -> get_string(1));
                if (godot::Color::html_is_valid(value)) throw Vital::Error::fetch("invalid-arguments");
                Vital::Godot::Core::get_environment() -> set_fog_light_color(godot::Color::html(value));
            }
            else {
                if (vm -> get_arg_count() < 4) throw Vital::Error::fetch("invalid-arguments");
                for (int i = 1; i <= 4; i++) {
                    if (!vm -> is_number(i)) throw Vital::Error::fetch("invalid-arguments");
                }
                Vital::Godot::Core::get_environment() -> set_fog_light_color(godot::Color(
                    vm -> get_float(1), 
                    vm -> get_float(2), 
                    vm -> get_float(3), 
                    vm -> get_float(4)
                ));
            }
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "get_light_color", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            auto value = Vital::Godot::Core::get_environment() -> get_fog_light_color();
            vm -> push_number(value.r);
            vm -> push_number(value.g);
            vm -> push_number(value.b);
            vm -> push_number(value.a);
            return 4;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "set_light_energy", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_fog_light_energy(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "get_light_energy", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_fog_light_energy());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "set_sun_scatter", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_fog_sun_scatter(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "get_sun_scatter", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_fog_sun_scatter());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "set_density", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_fog_density(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "get_density", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_fog_density());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "set_height", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_fog_height(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "get_height", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_fog_height());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "set_height_density", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_fog_height_density(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "get_height_density", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_fog_height_density());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "set_aerial_perspective", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_fog_aerial_perspective(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "get_aerial_perspective", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_fog_aerial_perspective());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "set_sky_affect", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_fog_sky_affect(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "get_sky_affect", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_fog_sky_affect());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "set_depth_curve", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_fog_depth_curve(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "get_depth_curve", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_fog_depth_curve());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "set_depth_begin", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_fog_depth_begin(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "get_depth_begin", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_fog_depth_begin());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "set_depth_end", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_fog_depth_end(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "fog", "get_depth_end", [](auto* ref) -> int {
        auto vm = Machine::fetch_machine(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_fog_depth_end());
            return 1;
        });
    });
    #endif
}