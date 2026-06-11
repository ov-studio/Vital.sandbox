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
        
            API::bind(vm, {base_name}, "is_loaded", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                vm -> push_value(Manager::Resource::get_singleton() -> is_loaded(vm -> get_string(1)));
                return 1;
            });
        
            API::bind(vm, {base_name}, "is_running", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                vm -> push_value(Manager::Resource::get_singleton() -> is_running(vm -> get_string(1)));
                return 1;
            });
        
            API::bind(vm, {base_name}, "get_list", [](auto vm, auto& id) -> int {
                auto resources = Manager::Resource::get_singleton() -> get_all_resources();
                vm -> create_table();
                int i = 1;
                for (const auto& manifest : resources) {
                    vm -> push_value(manifest -> ref);
                    vm -> set_table_field(i++, -2);
                }
                return 1;
            });
        
            #if !defined(VSDK_Client)
            API::bind(vm, {base_name}, "start", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                vm -> push_value(Manager::Resource::get_singleton() -> start(vm -> get_string(1)));
                return 1;
            });
        
            API::bind(vm, {base_name}, "stop", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                vm -> push_value(Manager::Resource::get_singleton() -> stop(vm -> get_string(1)));
                return 1;
            });
        
            API::bind(vm, {base_name}, "restart", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                vm -> push_value(Manager::Resource::get_singleton() -> restart(vm -> get_string(1)));
                return 1;
            });
            #endif
        }
    };
}