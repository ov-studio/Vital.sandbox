/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: private: api.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: API Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/lua/public/api.h>
#include <Vital.sandbox/Sandbox/lua/api/public/engine.h>
#include <Vital.sandbox/Sandbox/lua/api/public/coroutine.h>
#include <Vital.sandbox/Sandbox/lua/api/public/file.h>
#include <Vital.sandbox/Sandbox/lua/api/public/crypto.h>
#include <Vital.sandbox/Sandbox/lua/api/public/rest.h>
#include <Vital.sandbox/Sandbox/lua/api/public/network.h>


///////////////////////////////
// Vital: Sandbox: Lua: API //
///////////////////////////////

namespace Vital::Sandbox::Lua {
    void create::hook(const std::string& mode) {
        auto instance = static_cast<void*>(this);
        vsdk_apis natives = {
            {API::Engine::bind, API::Engine::inject},
            {API::Coroutine::bind, API::Coroutine::inject},
            {API::File::bind, API::File::inject},
            {API::Crypto::bind, API::Crypto::inject},
            {API::REST::bind, API::REST::inject},
            {API::Network::bind, API::Network::inject}
        };
        for (auto& i : apis) natives.push_back(i);
        for (auto& i : natives) {
            if (mode == "bind") i.first(instance);
            else if (mode == "inject") i.second(instance);
        }
    }

    namespace API {
        std::function<void(const std::string&)> vsdk_errorhandle = NULL;
        void createErrorHandle(std::function<void(const std::string&)> exec) {
            vsdk_errorhandle = exec;
        }
    
        void error(const std::string& error) {
            if (!vsdk_errorhandle) return;
            vsdk_errorhandle(error);
        }
    
        void bind(vsdk_vm* vm, const std::string& parent, const std::string& name, vsdk_exec exec) {
            vm -> registerFunction(name, exec, parent);
        }
    }
}