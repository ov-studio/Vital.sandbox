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
#include <Sandbox/lua/module/public/engine.h>
#include <Sandbox/lua/module/public/coroutine.h>
#include <Sandbox/lua/module/public/file.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    std::map<vsdk_bind, vsdk_exec> vsdk_binds;
    std::function<void(const std::string&)> vsdk_errorhandle = NULL;

    // Handlers //
    bool createErrorHandle(std::function<void(const std::string&)> exec) {
        vsdk_errorhandle = exec;
        return true;
    }
    bool error(const std::string& error) {
        if (!vsdk_errorhandle) return false;
        vsdk_errorhandle(error);
        return true;
    }
    bool bind(const std::string& parent, const std::string& name, vsdk_exec exec) {
        vsdk_bind ref = {parent, name};
        vsdk_binds.emplace(ref, exec);
        return true;
    }
    bool unbind(const std::string& parent, const std::string& name) {
        vsdk_bind ref = {parent, name};
        vsdk_binds.erase(ref);
        return true;
    }

    // Booter //
    bool boot() {
        Engine::boot();
        Coroutine::boot();
        File::boot();
        vSandbox_Crypto();
        vSandbox_Network();
        vSandbox_Audio();
        return true;
    }
}