/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: public: index.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <pch.h>
extern "C" {
    #include <Vendor/lua/lua.h>
    #include <Vendor/lua/lauxlib.h>
    #include <Vendor/lua/lualib.h>
}



/////////////////////
// Namespace: Lua //
/////////////////////

namespace Vital::Lua {
    static std::vector<std::string> vBlacklist;
    static std::vector<luaL_Reg> vLibraries;
    static std::vector<std::string> vModules;
    static bool boot();
}