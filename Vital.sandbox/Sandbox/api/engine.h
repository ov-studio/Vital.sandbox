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
        inline static const std::string base_name = "engine";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "get_tick", [](auto vm) -> int {
                vm -> push_number(static_cast<int>(get_tick()));
                return 1;
            });
        
            API::bind(vm, {base_name}, "get_platform", [](auto vm) -> int {
                vm -> push_string(get_platform());
                return 1;
            });

            API::bind(vm, {base_name}, "get_timestamp", [](auto vm) -> int {
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
            API::bind(vm, {base_name}, "get_serial", [](auto vm) -> int {
                vm -> push_string(Vital::Tool::Inspect::fingerprint());
                return 1;
            });
            #endif
        
            API::bind(vm, {base_name}, "compile_string", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto input = vm -> get_string(1);
                auto result = vm -> compile_string(input);
                vm -> push_bool(result);
                return 1;
            });

            API::bind(vm, {base_name}, "load_string", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto input = vm -> get_string(1);
                bool auto_load = vm -> is_bool(2) ? vm -> get_bool(2) : true;
                bool use_env = vm -> is_bool(3) ? vm -> get_bool(3) : false;
                if (use_env && !vm -> is_table(4)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                int results = vm -> load_string(input, auto_load, use_env, 4);
                if (results == 0) vm -> push_bool(false);
                return results == 0 ? 1 : results;
            });

            API::bind(vm, {base_name}, "print", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1)) || (!Vital::Log::is_type(vm -> get_string(1)))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                std::string type = vm -> get_string(1);
                std::ostringstream buffer;
                for (int i = 2; i <= vm -> get_arg_count(); ++i) {
                    if (i != 2) buffer << " ";
                    buffer << vm -> to_string(i);
                }
                Vital::print(type, buffer.str());
                vm -> push_bool(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            #if defined(Vital_SDK_Client)
            vm -> get_global(base_name);
            vm -> get_table_field("print", -1);
            vm -> push_global("print");
            vm -> pop(1);
            #endif
            
            vm -> table_set_nil("dump", "string");
            vm -> table_set_nil("dump", "utf8");
        }
    };
}
