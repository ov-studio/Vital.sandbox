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
#include <Vital.sandbox/Sandbox/lua/public/vm.h>
#include <Vital.sandbox/Sandbox/lua/api/engine.h>
#include <Vital.sandbox/Sandbox/lua/api/coroutine.h>
#include <Vital.sandbox/Sandbox/lua/api/file.h>
#include <Vital.sandbox/Sandbox/lua/api/crypto.h>
#include <Vital.sandbox/Sandbox/lua/api/rest.h>
#include <Vital.sandbox/Sandbox/lua/api/network.h>


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
            {API::Rest::bind, API::Rest::inject},
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
    
        void bind(create* vm, const std::string& parent, const std::string& name, lua_CFunction exec) {
            vm -> registerFunction(name, exec, parent);
        }
    }
}