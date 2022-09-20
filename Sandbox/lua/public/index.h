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
#include <vector>
extern "C" {
    #include <Vendor/lua/lua.h>
    #include <Vendor/lua/lauxlib.h>
    #include <Vendor/lua/lualib.h>
}



/////////////////////
// Namespace: Lua //
/////////////////////

namespace Vital::Lua {
    std::vector<std::string> vBlacklist;
    std::vector<luaL_Reg> Library_Whitelist;
    std::vector<std::string> vModules;
}