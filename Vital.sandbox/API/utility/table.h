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
        }

        static void bind(Machine* vm) {
            
        }
    };
}