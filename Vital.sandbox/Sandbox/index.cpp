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
        {API::Engine::bind, API::Engine::inject},
        {API::Coroutine::bind, API::Coroutine::inject},
        {API::Network::bind, API::Network::inject},
        {API::Rest::bind, API::Rest::inject},
        {API::File::bind, API::File::inject},
        {API::Crypto::bind, API::Crypto::inject},
        {API::Shrinker::bind, API::Shrinker::inject}
    };

    namespace API {
        void log(const std::string& type, const std::string& message) {
            Vital::print(type, message);
        }
    
        void bind(Machine* vm, const std::string& nspace, const std::string& name, vm_bind exec) {
            vm -> bind_function(nspace, name, std::move(exec));
        }
    }
}