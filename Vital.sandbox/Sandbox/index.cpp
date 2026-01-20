/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: index.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Root Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>
#include <Vital.sandbox/Sandbox/api/engine.h>
#include <Vital.sandbox/Sandbox/api/coroutine.h>
#include <Vital.sandbox/Sandbox/api/file.h>
#include <Vital.sandbox/Sandbox/api/crypto.h>
#include <Vital.sandbox/Sandbox/api/rest.h>
#include <Vital.sandbox/Sandbox/api/network.h>


//////////////////////////
// Vital: Sandbox: API //
//////////////////////////

namespace Vital::Sandbox {
    vm_apis Machine::natives = {
        {API::Engine::bind, API::Engine::inject},
        {API::Coroutine::bind, API::Coroutine::inject},
        {API::File::bind, API::File::inject},
        {API::Crypto::bind, API::Crypto::inject},
        {API::Rest::bind, API::Rest::inject},
        {API::Network::bind, API::Network::inject}
    };

    namespace API {
        void error(const std::string& error) {
            #if defined(Vital_SDK_Client)
            // TODO: Print it in console
            #else
            // TODO: Print it in cmd line
            #endif
        }
    
        void bind(Machine* vm, const std::string& nspace, const std::string& name, vm_exec exec) {
            vm -> table_set_function(name, exec, nspace);
        }
    }
}