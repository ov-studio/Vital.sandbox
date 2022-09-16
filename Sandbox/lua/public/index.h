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
    static const std::string vBlacklist[];
    static const luaL_Reg Library_Whitelist[];
    static const std::string vModules[];
}