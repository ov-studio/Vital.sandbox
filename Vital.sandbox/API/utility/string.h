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
    struct Engine : vm_module {
        inline static const std::vector<std::string> base_scope = {"util", "string"};

        static void bind(Machine* vm) {

        }

        static void inject(Machine* vm) {
            {
                vm -> table_set_nil("dump", "string");
                vm -> table_set_nil("dump", "utf8");
            }
        }
    };
}