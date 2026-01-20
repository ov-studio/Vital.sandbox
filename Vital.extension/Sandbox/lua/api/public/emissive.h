/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: emissive.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Emissive APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/index.h>


////////////////////////////////////
// Vital: Sandbox: API: Emissive //
////////////////////////////////////

namespace Vital::Sandbox::API {
    class Emissive : public Vital::Tool::Module {
        public:
            inline static void bind(void* machine) {
                auto vm = Machine::to_machine(machine);

                #if defined(Vital_SDK_Client)
                Vital::Sandbox::API::bind(vm, "emissive", "set_enabled", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_bool(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto state = vm -> get_bool(1);
                        Vital::Godot::Core::get_environment() -> set_glow_enabled(state);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "is_enabled", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_bool(Vital::Godot::Core::get_environment() -> is_glow_enabled());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "set_level_intensity", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 2) || (!vm -> is_number(1)) || (!vm -> is_number(2))) throw Vital::Error::fetch("invalid-arguments");
                        auto idx = vm -> get_int(1);
                        auto value = vm -> get_float(2);
                        Vital::Godot::Core::get_environment() -> set_glow_level(idx, value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "get_level_intensity", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto idx = vm -> get_int(1);
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_glow_level(idx));
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "set_normalized", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_bool(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto state = vm -> get_bool(1);
                        Vital::Godot::Core::get_environment() -> set_glow_normalized(state);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "is_normalized", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_bool(Vital::Godot::Core::get_environment() -> is_glow_normalized());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "set_intensity", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_glow_intensity(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "get_intensity", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_glow_intensity());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "set_strength", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_glow_strength(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "get_strength", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_glow_strength());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "set_mix", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_glow_mix(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "get_mix", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_glow_mix());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "set_bloom", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_glow_bloom(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "get_bloom", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_glow_bloom());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "set_blend_mode", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_int(1);
                        if ((value < godot::Environment::GLOW_BLEND_MODE_ADDITIVE) || (value > godot::Environment::GLOW_BLEND_MODE_MIX)) throw Vital::Error::fetch("invalid-arguments");
                        Vital::Godot::Core::get_environment() -> set_glow_blend_mode(static_cast<godot::Environment::GlowBlendMode>(value));
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "get_blend_mode", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_glow_blend_mode());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "set_hdr_bleed_threshold", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_glow_hdr_bleed_threshold(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "get_hdr_bleed_threshold", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_glow_hdr_bleed_threshold());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "set_hdr_bleed_scale", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_glow_hdr_bleed_scale(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "get_hdr_bleed_scale", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_glow_hdr_bleed_scale());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "set_hdr_luminance_cap", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_glow_hdr_luminance_cap(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "get_hdr_luminance_cap", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_glow_hdr_luminance_cap());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "set_map_strength", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_glow_map_strength(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "emissive", "get_map_strength", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_glow_map_strength());
                        return 1;
                    });
                });
                #endif
            }
    };
}