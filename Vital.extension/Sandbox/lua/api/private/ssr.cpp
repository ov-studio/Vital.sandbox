/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: private: ssr.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SSR APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/api/public/ssr.h>


///////////////////////////////////////////
// Vital: Godot: Sandbox: Lua: API: SSR //
///////////////////////////////////////////

void Vital::Godot::Sandbox::Lua::API::SSR::bind(void* instance) {
    auto vm = Vital::Sandbox::Lua::toVM(instance);

    #if defined(Vital_SDK_Client)
    Vital::Sandbox::Lua::API::bind(vm, "ssr", "setEnabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isBool(1))) throw Vital::Error::fetch("invalid-arguments");
            auto state = vm -> getBool(1);
            Vital::Godot::Core::get_environment() -> set_ssr_enabled(state);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssr", "isEnabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setBool(Vital::Godot::Core::get_environment() -> is_ssr_enabled());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssr", "setMaxSteps", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getInt(1);
            Vital::Godot::Core::get_environment() -> set_ssr_max_steps(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssr", "getMaxSteps", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_ssr_max_steps());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssr", "setFadeIn", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_ssr_fade_in(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssr", "getFadeIn", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_ssr_fade_in());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssr", "setFadeOut", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_ssr_fade_out(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssr", "getFadeOut", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_ssr_fade_out());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssr", "setDepthTolerance", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_ssr_depth_tolerance(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssr", "getDepthTolerance", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_ssr_depth_tolerance());
            return 1;
        });
    });
    #endif
}