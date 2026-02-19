/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: ssil.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SSIL APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/index.h>


////////////////////////////////
// Vital: Sandbox: API: SSIL //
////////////////////////////////

namespace Vital::Sandbox::API {
    class SSIL : public Vital::Tool::Module {
        public:
            inline static void bind(void* machine) {
                auto vm = Machine::to_machine(machine);

                #if defined(Vital_SDK_Client)
                Vital::Sandbox::API::bind(vm, "ssil", "set_enabled", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_bool(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                        auto state = vm -> get_bool(1);
                        Vital::Godot::Core::get_environment() -> set_ssil_enabled(state);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "ssil", "is_enabled", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_bool(Vital::Godot::Core::get_environment() -> is_ssil_enabled());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "ssil", "set_radius", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_ssil_radius(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "ssil", "get_radius", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_ssil_radius());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "ssil", "set_intensity", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_ssil_intensity(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "ssil", "get_intensity", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_ssil_intensity());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "ssil", "set_sharpness", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_ssil_sharpness(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "ssil", "get_sharpness", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_ssil_sharpness());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "ssil", "set_normal_rejection", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_ssil_normal_rejection(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "ssil", "get_normal_rejection", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_ssil_normal_rejection());
                        return 1;
                    });
                });
                #endif
            }
    };
}