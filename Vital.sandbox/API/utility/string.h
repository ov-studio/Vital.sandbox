/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: string.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: String APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>


/////////////////////////
// Vital: API: String //
/////////////////////////

namespace Vital::Sandbox::API {
    struct String : vm_module {
        inline static const std::vector<std::string> base_scope = {"util", "string"};
        inline static const std::vector<std::string> base_scope_utf8 = {"util", "utf8"};

        static void init(Machine* vm) {
            {
                vm -> scope_move_global(base_scope, "string", true);
                vm -> scope_nil_field(base_scope, "dump");
            }
            {
                vm -> scope_move_global(base_scope_utf8, "utf8", true);
                vm -> scope_nil_field(base_scope_utf8, "dump");
            }
            {
                vm -> create_namespace(base_scope[0]);
                int util_idx = vm -> get_count();
                vm -> get_table_field(base_scope.back(), util_idx);
                int string_idx = vm -> get_count();
                vm -> get_table_field(base_scope_utf8.back(), util_idx);
                int utf8_idx = vm -> get_count();
                if (vm -> is_table(string_idx) && vm -> is_table(utf8_idx)) {
                    vm -> push_nil();
                    while (vm -> next(utf8_idx)) {
                        vm -> push(-2);
                        vm -> push(-2);
                        vm -> set_table(string_idx);
                        vm -> pop(1);
                    }
                }
                vm -> push_nil();
                vm -> set_table_field(base_scope_utf8.back(), util_idx);
                vm -> pop(3);
            }
        }

        static void bind(Machine* vm) {

        }
    };
}