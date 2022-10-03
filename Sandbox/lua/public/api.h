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

namespace Vital::Sandbox::Lua::API {
    // Handlers //
    extern std::map<vital_exec_ref, vital_exec> vMethods;
    extern bool onErrorHandle(std::function<void(std::string&)> exec);

    // Helpers //
    extern bool error(std::string& error);
    extern bool bind(std::string parent, std::string name, vital_exec exec);
    extern bool unbind(std::string parent, std::string name);

    // Booter //
    extern bool boot();

    // Binds //
    extern void vSandbox_Engine();
    extern void vSandbox_Crypto();
    extern void vSandbox_File();
}