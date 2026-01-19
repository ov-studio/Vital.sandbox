/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: private: emissive.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Emissive APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/api/public/emissive.h>


////////////////////////////////////////////////
// Vital: Godot: Sandbox: Lua: API: Emissive //
////////////////////////////////////////////////

void Vital::Godot::Sandbox::Lua::API::Emissive::bind(void* instance) {
    auto vm = Vital::Sandbox::Lua::toVM(instance);

    #if defined(Vital_SDK_Client)
    Vital::Sandbox::Lua::API::bind(vm, "emissive", "set_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isBool(1))) throw Vital::Error::fetch("invalid-arguments");
            auto state = vm -> getBool(1);
            Vital::Godot::Core::get_environment() -> set_glow_enabled(state);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "is_enabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setBool(Vital::Godot::Core::get_environment() -> is_glow_enabled());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "set_level_intensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 2) || (!vm -> isNumber(1)) || (!vm -> isNumber(2))) throw Vital::Error::fetch("invalid-arguments");
            auto idx = vm -> getInt(1);
            auto value = vm -> getFloat(2);
            Vital::Godot::Core::get_environment() -> set_glow_level(idx, value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "get_level_intensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto idx = vm -> getInt(1);
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_level(idx));
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "set_normalized", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isBool(1))) throw Vital::Error::fetch("invalid-arguments");
            auto state = vm -> getBool(1);
            Vital::Godot::Core::get_environment() -> set_glow_normalized(state);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "is_normalized", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setBool(Vital::Godot::Core::get_environment() -> is_glow_normalized());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "set_intensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_intensity(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "get_intensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_intensity());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "set_strength", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_strength(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "get_strength", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_strength());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "set_mix", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_mix(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "get_mix", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_mix());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "set_bloom", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_bloom(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "get_bloom", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_bloom());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "set_blend_mode", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getInt(1);
            if ((value < godot::Environment::GLOW_BLEND_MODE_ADDITIVE) || (value > godot::Environment::GLOW_BLEND_MODE_MIX)) throw Vital::Error::fetch("invalid-arguments");
            Vital::Godot::Core::get_environment() -> set_glow_blend_mode(static_cast<godot::Environment::GlowBlendMode>(value));
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "get_blend_mode", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_blend_mode());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "set_hdr_bleed_threshold", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_hdr_bleed_threshold(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "get_hdr_bleed_threshold", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_hdr_bleed_threshold());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "set_hdr_bleed_scale", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_hdr_bleed_scale(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "get_hdr_bleed_scale", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_hdr_bleed_scale());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "set_hdr_luminance_cap", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_hdr_luminance_cap(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "get_hdr_luminance_cap", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_hdr_luminance_cap());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "set_map_strength", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_map_strength(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "get_map_strength", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_map_strength());
            return 1;
        });
    });
    #endif
}