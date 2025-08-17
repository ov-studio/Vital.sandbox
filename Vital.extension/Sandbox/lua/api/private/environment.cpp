/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: private: environment.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Environment APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/api/public/environment.h>


///////////////////////////////////////////////////
// Vital: Godot: Sandbox: Lua: API: Environment //
///////////////////////////////////////////////////

void Vital::Godot::Sandbox::Lua::API::Environment::bind(void* instance) {
    auto vm = static_cast<Vital::Sandbox::Lua::vsdk_vm*>(instance);

    Vital::Sandbox::Lua::API::bind(vm, "environment", "set_sky_custom_fov", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto fov = vm -> getInt(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_sky_custom_fov(fov);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "get_sky_custom_fov", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_sky_custom_fov());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "set_bg_energy_multiplier", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto energy = vm -> getInt(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_bg_energy_multiplier(energy);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "get_bg_energy_multiplier", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_bg_energy_multiplier());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "set_ssao_radius", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto radius = vm -> getInt(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_ssao_radius(radius);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "get_ssao_radius", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_ssao_radius());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "set_ssao_intensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto intensity = vm -> getInt(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_ssao_intensity(intensity);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "get_ssao_intensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_ssao_intensity());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "set_ssao_power", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto power = vm -> getInt(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_ssao_power(power);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "get_ssao_power", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_ssao_power());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "set_ssao_detail", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto detail = vm -> getInt(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_ssao_detail(detail);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "get_ssao_detail", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_ssao_detail());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "set_ssao_horizon", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto horizon = vm -> getInt(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_ssao_horizon(horizon);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "get_ssao_horizon", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_ssao_horizon());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "set_ssao_sharpness", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto sharpness = vm -> getInt(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_ssao_sharpness(sharpness);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "get_ssao_sharpness", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_ssao_sharpness());
            return 1;
        });
    });


    Vital::Sandbox::Lua::API::bind(vm, "environment", "set_ssao_direct_light_affect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto amount = vm -> getInt(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_ssao_direct_light_affect(amount);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "get_ssao_direct_light_affect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_ssao_direct_light_affect());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "set_ssao_ao_channel_affect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto amount = vm -> getInt(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_ssao_ao_channel_affect(amount);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "environment", "get_ssao_ao_channel_affect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_ssao_ao_channel_affect());
            return 1;
        });
    });
}