/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: engine.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Engine APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/console.h>


/////////////////////////
// Vital: API: Engine //
/////////////////////////

namespace Vital::Sandbox::API {
    struct Engine : vm_module {
        inline static const std::string base_name = "engine";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "get_version", [](auto vm, auto& id) -> int {
                vm -> push_value(Build.to_string());
                return 1;
            });

            API::bind(vm, {base_name}, "get_tick", [](auto vm, auto& id) -> int {
                vm -> push_value(Tool::get_tick());
                return 1;
            });

            API::bind(vm, {base_name}, "get_platform", [](auto vm, auto& id) -> int {
                vm -> push_value(Tool::get_platform());
                return 1;
            });

            API::bind(vm, {base_name}, "get_timestamp", [](auto vm, auto& id) -> int {
                auto timestamp = Tool::get_timestamp();
                vm -> create_table();
                for (auto& [key, value] : timestamp.object) {
                    vm -> table_set_value(key, value);
                }
                return 1;
            });

            #if defined(Vital_SDK_Client)
            API::bind(vm, {base_name}, "get_serial", [](auto vm, auto& id) -> int {
                vm -> push_value(Tool::Inspect::fingerprint());
                return 1;
            });
            #endif

            API::bind(vm, {base_name}, "compile_string", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(input, chunk_name)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_string);

                auto input = vm -> get_string(1);
                auto chunk_name = vm -> is_string(2) ? vm -> get_string(2) : "";
                auto result = vm -> compile_string(input, chunk_name);
                if (result.empty()) {
                    vm -> push_value(true);
                    vm -> push_value(false);
                }
                else {
                    vm -> push_value(false);
                    vm -> push_value(result);
                }
                return 2;
            });

            API::bind(vm, {base_name}, "load_string", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(input, chunk_name, auto_load, use_env, env)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_string)
                    .optional(3, &Machine::is_bool)
                    .optional(4, &Machine::is_bool)
                    .optional(5, &Machine::is_table);
            
                auto input = vm -> get_string(1);
                auto chunk_name = vm -> is_string(2) ? vm -> get_string(2) : "";
                bool auto_load = vm -> is_bool(3) ? vm -> get_bool(3) : true;
                bool use_env = vm -> is_bool(4) ? vm -> get_bool(4) : false;
                int results = vm -> load_string(input, chunk_name, auto_load, use_env, 5);
                if (results == 0) vm -> push_value(false);
                return results == 0 ? 1 : results;
            });

            API::bind(vm, {base_name}, "print", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(type, ...)")
                    .require(1, &Machine::is_string);

                std::string type = vm -> get_string(1);
                std::ostringstream buffer;
                for (int i = 2; i <= vm -> get_count(); ++i) {
                    if (i != 2) buffer << " ";
                    buffer << vm -> to_string(i);
                }
                Tool::print(type, buffer.str());
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> get_global(base_name);
            vm -> get_table_field("print", -1);
            vm -> push_global("print");
            vm -> pop(1);
            vm -> table_set_nil("dump", "string");
            vm -> table_set_nil("dump", "utf8");
        }
    };
}