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
#include <Vital.extension/Engine/public/console.h>


//////////////////////////////////
// Vital: Sandbox: API: Engine //
//////////////////////////////////

namespace Vital::Sandbox::API {
    struct Engine : vm_module {
        static void bind(Machine* vm) {
            API::bind(vm, "engine", "get_tick", [](auto* vm) -> int {
                vm -> push_number(static_cast<int>(get_tick()));
                return 1;
            });
        
            API::bind(vm, "engine", "get_platform", [](auto* vm) -> int {
                vm -> push_string(get_platform());
                return 1;
            });

            API::bind(vm, "engine", "get_timestamp", [](auto* vm) -> int {
                auto timestamp = get_timestamp();
                vm -> create_table();
                vm -> table_set_number("hour", timestamp.object["hour"].as<int32_t>());
                vm -> table_set_number("minute", timestamp.object["minute"].as<int32_t>());
                vm -> table_set_number("second", timestamp.object["second"].as<int32_t>());
                vm -> table_set_number("day", timestamp.object["day"].as<int32_t>());
                vm -> table_set_number("month", timestamp.object["month"].as<int32_t>());
                vm -> table_set_number("year", timestamp.object["year"].as<int32_t>());
                return 1;
            });

            #if defined(Vital_SDK_Client)
            API::bind(vm, "engine", "get_serial", [](auto* vm) -> int {
                vm -> push_string(Vital::Tool::Inspect::fingerprint());
                return 1;
            });
            #endif
        
            API::bind(vm, "engine", "load_string", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto input = vm -> get_string(1);
                bool autoload = !vm -> is_bool(2) ? true : vm -> get_bool(2);
                bool result = vm -> load_string(input, autoload);
                if (autoload) vm -> push_bool(result);
                vm -> push_bool(result);
                return 1;
            });

            API::bind(vm, "engine", "print", [](auto* vm) -> int {
                std::ostringstream buffer;
                for (int i = 0; i < vm -> get_arg_count(); ++i) {
                    if (i != 0) buffer << " ";
                    buffer << vm -> to_string(i + 1);
                }
                Vital::print("info", buffer.str());
                vm -> push_bool(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            #if defined(Vital_SDK_Client)
            vm -> get_global("engine");
            vm -> get_table_field("print", -1);
            vm -> push_global("print");
            vm -> pop(1);
            #endif
            
            vm -> table_set_nil("dump", "string");
            vm -> table_set_nil("dump", "utf8");
        }
    };
}
