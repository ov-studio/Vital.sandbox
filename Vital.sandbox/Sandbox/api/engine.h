/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: engine.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Engine APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>


//////////////////////////////////
// Vital: Sandbox: API: Engine //
//////////////////////////////////

namespace Vital::Sandbox::API {
    class Engine : public Vital::Tool::Module {
        public:
            inline static void bind(void* machine) {
                auto vm = Machine::to_machine(machine);

                API::bind(vm, "engine", "get_platform", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_string(get_platform());
                        return 1;
                    });
                });
            
                API::bind(vm, "engine", "get_tick", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_number(static_cast<int>(get_tick()));
                        return 1;
                    });
                });
            
                #if defined(Vital_SDK_Client)
                API::bind(vm, "engine", "get_serial", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        vm -> push_string(Vital::Tool::Inspect::fingerprint());
                        return 1;
                    });
                });
                #endif
            
                #if defined(Vital_SDK_Client)
                // TODO: MAKE IT OUTPUT TO GAME CONSOLE
                API::bind(vm, "engine", "print", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        std::ostringstream buffer;
                        for (int i = 0; i < vm -> get_arg_count(); ++i) {
                            size_t length;
                            const char* value = luaL_tolstring(ref, i + 1, &length);
                            if (i != 0) buffer << " ";
                            buffer << std::string(value, length);
                            vm -> pop(1);
                        }
                        godot::UtilityFunctions::print(to_godot_string(buffer.str()));
                        vm -> push_bool(true);
                        return 1;
                    });
                });
                #endif
            
                API::bind(vm, "engine", "load_string", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto buffer = vm -> get_string(1);
                        bool result = false;
                        if (vm -> is_bool(2)) {
                            bool autoload = vm -> get_bool(2);
                            result = vm -> load_string(buffer, autoload);
                            if (result) return 1;
                        }
                        else result = vm -> load_string(buffer);
                        vm -> push_bool(result);
                        return 1;
                    });
                });
            }

            inline static void inject(void* machine) {
                auto vm = Machine::to_machine(machine);

                #if defined(Vital_SDK_Client)
                vm -> get_global("engine");
                vm -> get_table_field("print", -1);
                vm -> push_global("print");
                vm -> pop(1);
                #endif
            }
    };
}