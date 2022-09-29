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
    #include <lua/lua.h>
    #include <lua/lauxlib.h>
    #include <lua/lualib.h>
}


/////////////////////
// Namespace: Lua //
/////////////////////

namespace Vital::Lua {
    extern std::vector<std::string> vBlacklist;
    extern std::vector<luaL_Reg> vLibraries;
    extern std::vector<std::string> vModules;
}