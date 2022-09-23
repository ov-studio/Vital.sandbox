/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: public: api.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
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

namespace Vital::Lua::API {
    extern bool boot();
    extern bool onErrorHandle(std::function<void(std::string&)> exec);
    extern bool error(std::string& error);
    extern bool bind(std::string parent, std::string name, std::function<int(vital_vm* vm)> exec);
    extern bool unbind(std::string parent, std::string name);

    // Binds //
    extern void vSandbox_Engine();
    extern void vSandbox_File();
}