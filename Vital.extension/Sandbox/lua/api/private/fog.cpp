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
    auto vm = static_cast<Vital::Sandbox::Lua::vsdk_vm*>(instance);

    #if defined(Vital_SDK_Client)
    Vital::Sandbox::Lua::API::bind(vm, "fog", "setEnabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isBool(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto state = vm -> getBool(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_fog_enabled(state);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "isEnabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setBool(Vital::Godot::Engine::Singleton::get_environment() -> is_fog_enabled());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "setLightColor", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if (vm -> isString(1)) {
                auto value = vm -> getString(1);
                //if (godot::Color::html_is_valid(value)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                //Vital::Godot::Engine::Singleton::get_environment() -> set_fog_light_color(godot::Color::html(value));
            }
            else {
                if (vm -> getArgCount() < 4) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                for (int i = 1; i <= 4; i++) {
                    if (!vm -> isNumber(i)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                }
                Vital::Godot::Engine::Singleton::get_environment() -> set_fog_light_color(godot::Color(
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

    Vital::Sandbox::Lua::API::bind(vm, "fog", "getLightColor", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            auto value = Vital::Godot::Engine::Singleton::get_environment() -> get_fog_light_color();
            vm -> setNumber(value.r);
            vm -> setNumber(value.g);
            vm -> setNumber(value.b);
            vm -> setNumber(value.a);
            return 4;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "setLightEnergy", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_fog_light_energy(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "getLightEnergy", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_fog_light_energy());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "setSunScatter", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_fog_sun_scatter(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "getSunScatter", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_fog_sun_scatter());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "setDensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_fog_density(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "getDensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_fog_density());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "setHeight", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_fog_height(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "getHeight", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_fog_height());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "setHeightDensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_fog_height_density(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "getHeightDensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_fog_height_density());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "setAerialPerspective", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_fog_aerial_perspective(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "getAerialPerspective", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_fog_aerial_perspective());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "setSkyAffect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_fog_sky_affect(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "getSkyAffect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_fog_sky_affect());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "setDepthCurve", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_fog_depth_curve(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "getDepthCurve", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_fog_depth_curve());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "setDepthBegin", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_fog_depth_begin(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "getDepthBegin", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_fog_depth_begin());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "setDepthEnd", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_fog_depth_end(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "fog", "getDepthEnd", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_fog_depth_end());
            return 1;
        });
    });
    #endif
}