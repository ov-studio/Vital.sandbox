/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: public: api.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: API Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/lua/public/vm.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    // Handlers //
    extern std::map<vsdk_bind, vsdk_exec> vsdk_binds;
    extern bool createErrorHandle(std::function<void(const std::string&)> exec);
    extern bool error(const std::string& error);
    extern bool bind(const std::string& parent, const std::string& name, vsdk_exec exec);
    extern bool unbind(const std::string& parent, const std::string& name);

    // Booter //
    extern bool boot();

    // Binds //
    extern void vSandbox_File();
    extern void vSandbox_Crypto();
    extern void vSandbox_Network();
    extern void vSandbox_Audio();
}