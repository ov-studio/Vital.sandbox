/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: sdfgi.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SDFGI APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/public/index.h>


/////////////////////////////////
// Vital: Sandbox: API: SDFGI //
/////////////////////////////////

namespace Vital::Sandbox::API {
    class SDFGI : public Vital::Tool::Module {
        public:
            inline static void bind(void* machine) {
                auto vm = Machine::to_machine(machine);

                #if defined(Vital_SDK_Client)
                Vital::Sandbox::API::bind(vm, "sdfgi", "set_enabled", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_bool(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto state = vm -> get_bool(1);
                        Vital::Godot::Core::get_environment() -> set_sdfgi_enabled(state);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "is_enabled", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_bool(Vital::Godot::Core::get_environment() -> is_sdfgi_enabled());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "set_cascades", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_int(1);
                        Vital::Godot::Core::get_environment() -> set_sdfgi_cascades(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "get_cascades", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_sdfgi_cascades());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "set_min_cell_size", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_sdfgi_min_cell_size(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "get_min_cell_size", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_sdfgi_min_cell_size());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "set_max_distance", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_sdfgi_max_distance(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "get_max_distance", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_sdfgi_max_distance());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "set_y_scale", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_int(1);
                        if ((value < godot::Environment::SDFGI_Y_SCALE_50_PERCENT) || (value > godot::Environment::SDFGI_Y_SCALE_100_PERCENT)) throw Vital::Error::fetch("invalid-arguments");
                        Vital::Godot::Core::get_environment() -> set_sdfgi_y_scale(static_cast<godot::Environment::SDFGIYScale>(value));
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "get_y_scale", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_sdfgi_y_scale());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "set_use_occlusion", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_bool(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto state = vm -> get_bool(1);
                        Vital::Godot::Core::get_environment() -> set_sdfgi_use_occlusion(state);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "is_using_occlusion", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_bool(Vital::Godot::Core::get_environment() -> is_sdfgi_using_occlusion());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "set_bounce_feedback", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_sdfgi_bounce_feedback(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "get_bounce_feedback", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_sdfgi_bounce_feedback());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "set_read_sky_light", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_bool(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto state = vm -> get_bool(1);
                        Vital::Godot::Core::get_environment() -> set_sdfgi_read_sky_light(state);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "is_reading_sky_light", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_bool(Vital::Godot::Core::get_environment() -> is_sdfgi_reading_sky_light());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "set_energy", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_sdfgi_energy(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "get_energy", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_sdfgi_energy());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "set_normal_bias", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_sdfgi_normal_bias(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "get_normal_bias", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_sdfgi_normal_bias());
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "set_probe_bias", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto value = vm -> get_float(1);
                        Vital::Godot::Core::get_environment() -> set_sdfgi_probe_bias(value);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                Vital::Sandbox::API::bind(vm, "sdfgi", "get_probe_bias", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(Vital::Godot::Core::get_environment() -> get_sdfgi_probe_bias());
                        return 1;
                    });
                });
                #endif
            }
    };
}