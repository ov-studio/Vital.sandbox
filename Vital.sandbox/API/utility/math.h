/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: math.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Math APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>


///////////////////////
// Vital: API: Math //
///////////////////////

namespace Vital::Sandbox::API {
    struct Math : vm_module {
        inline static const std::vector<std::string> base_scope = {"util", "string"};

        static void init(Machine* vm) {
            vm -> scope_move_global({"util", "math"}, "math", true);
        }

        static void bind(Machine* vm) {
            
        }
    };
}