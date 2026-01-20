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
    auto vm = Vital::Sandbox::Lua::to_vm(instance);

    #if defined(Vital_SDK_Client)
    Vital::Sandbox::API::bind(vm, "ssr", "set_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_bool(1))) throw Vital::Error::fetch("invalid-arguments");
            auto state = vm -> get_bool(1);
            Vital::Godot::Core::get_environment() -> set_ssr_enabled(state);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "ssr", "is_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> push_bool(Vital::Godot::Core::get_environment() -> is_ssr_enabled());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "ssr", "set_max_steps", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_int(1);
            Vital::Godot::Core::get_environment() -> set_ssr_max_steps(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "ssr", "get_max_steps", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_ssr_max_steps());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "ssr", "set_fade_in", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_ssr_fade_in(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "ssr", "get_fade_in", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_ssr_fade_in());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "ssr", "set_fade_out", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_ssr_fade_out(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "ssr", "get_fade_out", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_ssr_fade_out());
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "ssr", "set_depth_tolerance", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_ssr_depth_tolerance(value);
            vm -> push_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::API::bind(vm, "ssr", "get_depth_tolerance", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> push_number(Vital::Godot::Core::get_environment() -> get_ssr_depth_tolerance());
            return 1;
        });
    });
    #endif
}