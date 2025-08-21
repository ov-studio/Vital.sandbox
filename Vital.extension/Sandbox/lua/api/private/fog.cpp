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
    #endif
}