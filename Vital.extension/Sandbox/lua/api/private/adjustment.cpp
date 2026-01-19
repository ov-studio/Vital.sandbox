/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: private: adjustment.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Adjustment APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/api/public/adjustment.h>


//////////////////////////////////////////////////
// Vital: Godot: Sandbox: Lua: API: Adjustment //
//////////////////////////////////////////////////

void Vital::Godot::Sandbox::Lua::API::Adjustment::bind(void* instance) {
    auto vm = Vital::Sandbox::Lua::toVM(instance);

    #if defined(Vital_SDK_Client)
    Vital::Sandbox::Lua::API::bind(vm, "adjustment", "set_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isBool(1))) throw Vital::Error::fetch("invalid-arguments");
            auto state = vm -> getBool(1);
            Vital::Godot::Core::get_environment() -> set_adjustment_enabled(state);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "adjustment", "is_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setBool(Vital::Godot::Core::get_environment() -> is_adjustment_enabled());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "adjustment", "set_brightness", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_adjustment_brightness(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "adjustment", "get_brightness", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_adjustment_brightness());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "adjustment", "set_contrast", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_adjustment_contrast(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "adjustment", "get_contrast", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_adjustment_contrast());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "adjustment", "set_saturation", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_adjustment_saturation(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "adjustment", "get_saturation", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_adjustment_saturation());
            return 1;
        });
    });
    #endif
}