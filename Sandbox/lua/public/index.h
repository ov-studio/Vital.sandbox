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
extern "C" {
    #include <Vendor/lua/lua.h>
    #include <Vendor/lua/lauxlib.h>
    #include <Vendor/lua/lualib.h>
}



/////////////////////
// Namespace: Lua //
/////////////////////

namespace Vital::Lua {
    std::string vBlacklist[];
    luaL_Reg Library_Whitelist[];
    std::string vModules[];
}