/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: index.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>
#include <Vital.sandbox/Sandbox/api.h>
#include <Vital.extension/Engine/public/console.h>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    vm_apis Machine::internal_apis = {
        vm_module::make_api<API::Engine>(),
        vm_module::make_api<API::Database>(),
        vm_module::make_api<API::Discord>(),
        vm_module::make_api<API::Coroutine>(),
        vm_module::make_api<API::Network>(),
        vm_module::make_api<API::Rest>(),
        vm_module::make_api<API::File>(),
        vm_module::make_api<API::Crypto>(),
        vm_module::make_api<API::Shrinker>()
    };

    namespace API {
        void log(const std::string& type, const std::string& message) {
            Vital::print(type, message);
        }
    
        void bind(Machine* vm, const std::vector<std::string>& scope, const std::string& name, vm_bind exec) {
            vm -> bind(scope, name, std::move(exec));
        }
    }
}