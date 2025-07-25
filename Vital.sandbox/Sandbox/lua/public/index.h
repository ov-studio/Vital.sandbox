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
#include <Vendor/lua/lua.hpp>


/////////////////////
// Namespace: Lua //
/////////////////////

namespace Vital::Sandbox::Lua {
    extern std::vector<luaL_Reg> vsdk_libraries;
    extern std::vector<std::string> vsdk_blacklist;
}