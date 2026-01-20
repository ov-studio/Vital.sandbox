/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: API Utilities
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
        std::function<void(const std::string&)> vsdk_errorhandle = NULL;
        void create_error_handle(std::function<void(const std::string&)> exec) {
            vsdk_errorhandle = exec;
        }
    
        void error(const std::string& error) {
            if (!vsdk_errorhandle) return;
            vsdk_errorhandle(error);
        }
    
        void bind(Machine* vm, const std::string& nspace, const std::string& name, vm_exec exec) {
            vm -> table_set_function(name, exec, nspace);
        }
    }
}