/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: resource.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Resource APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Manager/public/resource.h>


///////////////////////////
// Vital: API: Resource //
///////////////////////////

namespace Vital::Sandbox::API {
    struct Resource : vm_module {
        inline static const std::string base_name = "resource";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "current", [](auto vm, auto& id) -> int {
                auto name = Manager::Resource::get_resource_from_vm(vm);
                if (name.empty()) vm -> push_value(false);
                else vm -> push_value(name);
                return 1;
            });
        }
    };
}