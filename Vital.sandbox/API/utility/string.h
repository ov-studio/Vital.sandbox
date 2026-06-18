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
            // TODO: MERGE UTF8 AND STRING IN ONE IN HERE DIRECTLY? and then nil utf8? to avoid lua injection
        }

        static void bind(Machine* vm) {

        }
    };
}