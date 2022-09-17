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
#include "Vendor/lua/lua.hpp"


/////////////////////
// Namespace: Lua //
/////////////////////

namespace Vital::Lua {
    std::string vBlacklist[];
    luaL_Reg Library_Whitelist[];
    std::string vModules[];
}