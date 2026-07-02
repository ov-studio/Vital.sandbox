/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: table.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Table APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>


////////////////////////
// Vital: API: Table //
////////////////////////

namespace Vital::Sandbox::API {
    struct Table : vm_module {
        inline static const std::vector<std::string> base_scope = {"util", "table"};

        static void init(Machine* vm) {
            vm -> scope_move_global(base_scope, "table", true);
            {
                vm -> get_global("json");
                vm -> set_reference("sandbox", "json", -1);
                vm -> pop(1);
                vm -> push_nil();
                vm -> push_global("json");
            }
        }
        
        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "encode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(input)")
                    .require(1, &Machine::is_table);
        
                vm -> get_reference("sandbox", "json", true);
                vm -> get_table_field("encode", -1);
                vm -> rotate(-2, 1);
                vm -> pop(1);
                vm -> push(1);
                if (!vm -> pcall(1, 1)) vm -> push_value(false);
                return 1;
            });
        
            API::bind(vm, base_scope, "decode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(input)")
                    .require(1, &Machine::is_string);
        
                vm -> get_reference("sandbox", "json", true);
                vm -> get_table_field("decode", -1);
                vm -> rotate(-2, 1);
                vm -> pop(1);
                vm -> push(1);
                if (!vm -> pcall(1, 1)) vm -> push_value(false);
                return 1;
            });
        }
    };
}