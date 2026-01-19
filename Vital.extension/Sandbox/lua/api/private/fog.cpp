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


///////////////////////////////////////////
// Vital: Godot: Sandbox: Lua: API: Fog //
///////////////////////////////////////////

void Vital::Godot::Sandbox::Lua::API::Fog::bind(void* instance) {
    auto vm = Vital::Sandbox::Lua::create::toVM(instance);

    #if defined(Vital_SDK_Client)
    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isBool(1))) throw Vital::Error::fetch("invalid-arguments");
            auto state = vm -> getBool(1);
            Vital::Godot::Core::get_environment() -> set_fog_enabled(state);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "is_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setBool(Vital::Godot::Core::get_environment() -> is_fog_enabled());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_mode", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getInt(1);
            if ((value < godot::Environment::FOG_MODE_EXPONENTIAL) || (value > godot::Environment::FOG_MODE_DEPTH)) throw Vital::Error::fetch("invalid-arguments");
            Vital::Godot::Core::get_environment() -> set_fog_mode(static_cast<godot::Environment::FogMode>(value));
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "get_mode", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_fog_mode());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_light_color", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if (vm -> isString(1)) {
                auto value = to_godot_string(vm -> getString(1));
                if (godot::Color::html_is_valid(value)) throw Vital::Error::fetch("invalid-arguments");
                Vital::Godot::Core::get_environment() -> set_fog_light_color(godot::Color::html(value));
            }
            else {
                if (vm -> getArgCount() < 4) throw Vital::Error::fetch("invalid-arguments");
                for (int i = 1; i <= 4; i++) {
                    if (!vm -> isNumber(i)) throw Vital::Error::fetch("invalid-arguments");
                }
                Vital::Godot::Core::get_environment() -> set_fog_light_color(godot::Color(
                    vm -> getFloat(1), 
                    vm -> getFloat(2), 
                    vm -> getFloat(3), 
                    vm -> getFloat(4)
                ));
            }
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "get_light_color", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            auto value = Vital::Godot::Core::get_environment() -> get_fog_light_color();
            vm -> setNumber(value.r);
            vm -> setNumber(value.g);
            vm -> setNumber(value.b);
            vm -> setNumber(value.a);
            return 4;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_light_energy", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_fog_light_energy(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "get_light_energy", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_fog_light_energy());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_sun_scatter", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_fog_sun_scatter(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "get_sun_scatter", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_fog_sun_scatter());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_density", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_fog_density(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "get_density", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_fog_density());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_height", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_fog_height(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "get_height", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_fog_height());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_height_density", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_fog_height_density(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "get_height_density", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_fog_height_density());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_aerial_perspective", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_fog_aerial_perspective(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "get_aerial_perspective", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_fog_aerial_perspective());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_sky_affect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_fog_sky_affect(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "get_sky_affect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_fog_sky_affect());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_depth_curve", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_fog_depth_curve(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "get_depth_curve", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_fog_depth_curve());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_depth_begin", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_fog_depth_begin(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "get_depth_begin", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_fog_depth_begin());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "set_depth_end", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_fog_depth_end(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "get_depth_end", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_fog_depth_end());
            return 1;
        });
    });
    #endif
}