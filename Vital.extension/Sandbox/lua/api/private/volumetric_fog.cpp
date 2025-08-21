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
    auto vm = static_cast<Vital::Sandbox::Lua::vsdk_vm*>(instance);

    #if defined(Vital_SDK_Client)
    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "setEnabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isBool(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto state = vm -> getBool(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_enabled(state);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "isEnabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setBool(Vital::Godot::Engine::Singleton::get_environment() -> is_volumetric_fog_enabled());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "setEmission", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if (vm -> isString(1)) {
                godot::String value(vm -> getString(1).c_str());
                if (godot::Color::html_is_valid(value)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_emission(godot::Color::html(value));
            }
            else {
                if (vm -> getArgCount() < 4) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                for (int i = 1; i <= 4; i++) {
                    if (!vm -> isNumber(i)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                }
                Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_emission(godot::Color(
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

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "getEmission", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            auto value = Vital::Godot::Engine::Singleton::get_environment() -> get_volumetric_fog_emission();
            vm -> setNumber(value.r);
            vm -> setNumber(value.g);
            vm -> setNumber(value.b);
            vm -> setNumber(value.a);
            return 4;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "setAlbedo", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if (vm -> isString(1)) {
                godot::String value(vm -> getString(1).c_str());
                if (godot::Color::html_is_valid(value)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_albedo(godot::Color::html(value));
            }
            else {
                if (vm -> getArgCount() < 4) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                for (int i = 1; i <= 4; i++) {
                    if (!vm -> isNumber(i)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                }
                Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_albedo(godot::Color(
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

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "getAlbedo", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            auto value = Vital::Godot::Engine::Singleton::get_environment() -> get_volumetric_fog_albedo();
            vm -> setNumber(value.r);
            vm -> setNumber(value.g);
            vm -> setNumber(value.b);
            vm -> setNumber(value.a);
            return 4;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "setDensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_density(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "getDensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_volumetric_fog_density());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "setEmissionEnergy", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_emission_energy(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "getEmissionEnergy", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_volumetric_fog_emission_energy());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "setAnisotropy", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_anisotropy(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "getAnisotropy", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_volumetric_fog_anisotropy());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "setLength", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_length(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "getLength", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_volumetric_fog_length());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "setDetailSpread", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_detail_spread(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "getDetailSpread", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_volumetric_fog_detail_spread());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "setGIInject", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_gi_inject(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "getGIInject", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_volumetric_fog_gi_inject());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "setAmbientInject", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_ambient_inject(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "getAmbientInject", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_volumetric_fog_ambient_inject());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "setSkyAffect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto value = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_sky_affect(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "getSkyAffect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_volumetric_fog_sky_affect());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "setTemporalReprojectionEnabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isBool(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto state = vm -> getBool(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_volumetric_fog_temporal_reprojection_enabled(state);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "volumetric_fog", "getTemporalReprojectionEnabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setBool(Vital::Godot::Engine::Singleton::get_environment() -> is_volumetric_fog_temporal_reprojection_enabled());
            return 1;
        });
    });
    #endif
}