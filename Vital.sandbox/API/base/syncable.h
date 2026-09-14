/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: base: syncable.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Syncable APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/syncable.h>


///////////////////////////
// Vital: API: Syncable //
///////////////////////////

namespace Vital::Sandbox::API {
    struct Syncable {
        template<typename Instance>
        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "get_net_id", [](auto vm, auto self, auto& id) -> int {
                auto* node = self -> get_node();
                if (node && node -> is_replicated()) vm -> push_value((int)node -> get_net_id());
                else vm -> push_value(false);
                return 1;
            });
        }
    };
}
