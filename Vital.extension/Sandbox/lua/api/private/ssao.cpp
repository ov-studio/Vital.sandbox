/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: private: ssao.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SSAO APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/api/public/ssao.h>


////////////////////////////////////////////
// Vital: Godot: Sandbox: Lua: API: SSAO //
////////////////////////////////////////////

void Vital::Godot::Sandbox::Lua::API::SSAO::bind(void* instance) {
    auto vm = Vital::Sandbox::Lua::create::to_vm(instance);

    #if defined(Vital_SDK_Client)
    Vital::Sandbox::Lua::API::bind(vm, "ssao", "set_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_bool(1))) throw Vital::Error::fetch("invalid-arguments");
            auto state = vm -> get_bool(1);
            Vital::Godot::Core::get_environment() -> set_ssao_enabled(state);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "is_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_bool(Vital::Godot::Core::get_environment() -> is_ssao_enabled());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "set_radius", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_ssao_radius(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "get_radius", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_ssao_radius());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "set_intensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_ssao_intensity(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "get_intensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_ssao_intensity());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "set_power", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_ssao_power(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "get_power", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_ssao_power());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "set_detail", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_ssao_detail(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "get_detail", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_ssao_detail());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "set_horizon", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_ssao_horizon(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "get_horizon", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_ssao_horizon());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "set_sharpness", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_ssao_sharpness(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "get_sharpness", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_ssao_sharpness());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "set_direct_light_affect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_ssao_direct_light_affect(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "get_direct_light_affect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_ssao_direct_light_affect());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "set_channel_affect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> get_float(1);
            Vital::Godot::Core::get_environment() -> set_ssao_ao_channel_affect(value);
            vm -> set_bool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "ssao", "get_channel_affect", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetch_vm(ref);
        return vm -> execute([&]() -> int {
            vm -> set_number(Vital::Godot::Core::get_environment() -> get_ssao_ao_channel_affect());
            return 1;
        });
    });
    #endif
}