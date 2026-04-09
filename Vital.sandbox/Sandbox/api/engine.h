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
#include <Vital.sandbox/Engine/public/console.h>


//////////////////////////////////
// Vital: Sandbox: API: Engine //
//////////////////////////////////

namespace Vital::Sandbox::API {
    struct Engine : vm_module {
        inline static const std::string base_name = "engine";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "get_version", [](auto vm) -> int {
                vm -> push_value(Build.to_string());
                return 1;
            });

            API::bind(vm, {base_name}, "get_tick", [](auto vm) -> int {
                vm -> push_value(get_tick());
                return 1;
            });
        
            API::bind(vm, {base_name}, "get_platform", [](auto vm) -> int {
                vm -> push_value(get_platform());
                return 1;
            });

            API::bind(vm, {base_name}, "get_timestamp", [](auto vm) -> int {
                auto timestamp = get_timestamp();
                vm -> create_table();
                for (auto& [key, value] : timestamp.object) {
                    vm -> table_set_value(key, value);
                }
                return 1;
            });

            #if defined(Vital_SDK_Client)
            API::bind(vm, {base_name}, "get_serial", [](auto vm) -> int {
                vm -> push_value(Vital::Tool::Inspect::fingerprint());
                return 1;
            });
            #endif
        
            API::bind(vm, {base_name}, "compile_string", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto input = vm -> get_string(1);
                auto result = vm -> compile_string(input);
                vm -> push_value(result);
                return 1;
            });

            API::bind(vm, {base_name}, "load_string", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto input = vm -> get_string(1);
                bool auto_load = vm -> is_bool(2) ? vm -> get_bool(2) : true;
                bool use_env = vm -> is_bool(3) ? vm -> get_bool(3) : false;
                if (use_env && !vm -> is_table(4)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                int results = vm -> load_string(input, auto_load, use_env, 4);
                if (results == 0) vm -> push_value(false);
                return results == 0 ? 1 : results;
            });

            API::bind(vm, {base_name}, "print", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                std::string type = vm -> get_string(1);
                std::ostringstream buffer;
                for (int i = 2; i <= vm -> get_count(); ++i) {
                    if (i != 2) buffer << " ";
                    buffer << vm -> to_string(i);
                }
                Vital::print(type, buffer.str());
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
