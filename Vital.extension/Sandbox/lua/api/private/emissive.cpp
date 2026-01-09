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
    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setEnabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isBool(1))) throw std::runtime_error(get_error("invalid-arguments"));
            auto state = vm -> getBool(1);
            Vital::Godot::Core::get_environment() -> set_glow_enabled(state);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "isEnabled", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setBool(Vital::Godot::Core::get_environment() -> is_glow_enabled());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setLevelIntensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 2) || (!vm -> isNumber(1)) || (!vm -> isNumber(2))) throw std::runtime_error(get_error("invalid-arguments"));
            auto idx = vm -> getInt(1);
            auto value = vm -> getFloat(2);
            Vital::Godot::Core::get_environment() -> set_glow_level(idx, value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getLevelIntensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(get_error("invalid-arguments"));
            auto idx = vm -> getInt(1);
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_level(idx));
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setNormalized", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isBool(1))) throw std::runtime_error(get_error("invalid-arguments"));
            auto state = vm -> getBool(1);
            Vital::Godot::Core::get_environment() -> set_glow_normalized(state);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "isNormalized", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setBool(Vital::Godot::Core::get_environment() -> is_glow_normalized());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setIntensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(get_error("invalid-arguments"));
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_intensity(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getIntensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_intensity());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setStrength", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(get_error("invalid-arguments"));
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_strength(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getStrength", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_strength());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setMix", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(get_error("invalid-arguments"));
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_mix(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getMix", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_mix());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setBloom", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(get_error("invalid-arguments"));
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_bloom(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getBloom", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_bloom());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setBlendMode", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(get_error("invalid-arguments"));
            auto value = vm -> getInt(1);
            if ((value < godot::Environment::GLOW_BLEND_MODE_ADDITIVE) || (value > godot::Environment::GLOW_BLEND_MODE_MIX)) throw std::runtime_error(get_error("invalid-arguments"));
            Vital::Godot::Core::get_environment() -> set_glow_blend_mode(static_cast<godot::Environment::GlowBlendMode>(value));
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getBlendMode", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_blend_mode());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setHDRBleedThreshold", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(get_error("invalid-arguments"));
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_hdr_bleed_threshold(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getHDRBleedThreshold", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_hdr_bleed_threshold());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setHDRBleedScale", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(get_error("invalid-arguments"));
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_hdr_bleed_scale(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getHDRBleedScale", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_hdr_bleed_scale());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setHDRLuminanceCap", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(get_error("invalid-arguments"));
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_hdr_luminance_cap(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getHDRLuminanceCap", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_hdr_luminance_cap());
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "setMapStrength", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(get_error("invalid-arguments"));
            auto value = vm -> getFloat(1);
            Vital::Godot::Core::get_environment() -> set_glow_map_strength(value);
            vm -> setBool(true);
            return 1;
        });
    });

    Vital::Sandbox::Lua::API::bind(vm, "emissive", "getMapStrength", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(Vital::Godot::Core::get_environment() -> get_glow_map_strength());
            return 1;
        });
    });
    #endif
}