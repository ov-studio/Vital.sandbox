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
    extern void createErrorHandle(std::function<void(const std::string&)> exec);
    extern void error(const std::string& error);
    extern void bind(const std::string& parent, const std::string& name, vsdk_exec exec);
    extern void boot(vsdk_vm* vm);
    extern void inject(vsdk_vm* vm);
}