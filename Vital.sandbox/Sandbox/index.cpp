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
    vm_apis Machine::natives = {
        API::Module::make<API::Engine>(),
        API::Module::make<API::Coroutine>(),
        API::Module::make<API::Network>(),
        API::Module::make<API::Rest>(),
        API::Module::make<API::File>(),
        API::Module::make<API::Crypto>(),
        API::Module::make<API::Shrinker>()
    };

    namespace API {
        void log(const std::string& type, const std::string& message) {
            Vital::print(type, message);
        }
    
        void bind(Machine* vm, const std::string& nspace, const std::string& name, vm_bind exec) {
            vm -> bind(nspace, name, std::move(exec));
        }
    }
}