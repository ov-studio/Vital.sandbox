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
    auto vm = static_cast<Vital::Sandbox::Lua::vsdk_vm*>(instance);

    #if defined(Vital_SDK_Client)
    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setEnabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isBool(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto state = vm -> getBool(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_glow_enabled(state);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "isEnabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setBool(Vital::Godot::Engine::Singleton::get_environment() -> is_glow_enabled());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setLevelIntensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 2) || (!vm -> isNumber(1)) || (!vm -> isNumber(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto idx = vm -> getInt(1);
            auto amount = vm -> getFloat(2);
            Vital::Godot::Engine::Singleton::get_environment() -> set_glow_level(idx, amount);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getLevelIntensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto idx = vm -> getInt(1);
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_glow_level(idx));
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setNormalized", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isBool(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto state = vm -> getBool(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_glow_normalized(state);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "isNormalized", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setBool(Vital::Godot::Engine::Singleton::get_environment() -> is_glow_normalized());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setIntensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto amount = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_glow_intensity(amount);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getIntensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_glow_intensity());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setStrength", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto amount = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_glow_strength(amount);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getStrength", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_glow_strength());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setMix", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto amount = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_glow_mix(amount);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getMix", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_glow_mix());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setBloom", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto amount = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_glow_bloom(amount);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getBloom", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_glow_bloom());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setHDRBleedThreshold", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto amount = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_glow_hdr_bleed_threshold(amount);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getHDRBleedThreshold", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_glow_hdr_bleed_threshold());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setHDRBleedScale", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto amount = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_glow_hdr_bleed_scale(amount);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getHDRBleedScale", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_glow_hdr_bleed_scale());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setHDRLuminanceCap", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            auto amount = vm -> getFloat(1);
            Vital::Godot::Engine::Singleton::get_environment() -> set_glow_hdr_luminance_cap(amount);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getHDRLuminanceCap", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Engine::Singleton::get_environment() -> get_glow_hdr_luminance_cap());
            return 1;
        });
    });
    #endif
}