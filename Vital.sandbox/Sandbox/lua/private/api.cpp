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
#include <Sandbox/lua/public/api.h>
#include <Sandbox/lua/api/public/engine.h>
#include <Sandbox/lua/api/public/coroutine.h>
#include <Sandbox/lua/api/public/file.h>
#include <Sandbox/lua/api/public/crypto.h>
#include <Sandbox/lua/api/public/network.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua {
    void create::bind() {
        auto instance = static_cast<void*>(this);
        API::Engine::bind(instance);
        API::Coroutine::bind(instance);
        API::File::bind(instance);
        API::Crypto::bind(instance);
        API::Network::bind(instance);
    }

    void create::inject() {
        auto instance = static_cast<void*>(this);
        API::Engine::inject(instance);
        API::Coroutine::inject(instance);
        API::File::inject(instance);
        API::Crypto::inject(instance);
        API::Network::inject(instance);
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