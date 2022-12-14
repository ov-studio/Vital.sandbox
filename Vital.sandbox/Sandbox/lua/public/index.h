/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: public: index.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>
extern "C" {
    #include <Vendor/lua/lua.h>
    #include <Vendor/lua/lauxlib.h>
    #include <Vendor/lua/lualib.h>
}


/////////////////////
// Namespace: Lua //
/////////////////////

namespace Vital::Sandbox::Lua {
    extern std::vector<std::string> blacklist;
    extern std::vector<luaL_Reg> library;
    extern std::vector<std::string> module;
}