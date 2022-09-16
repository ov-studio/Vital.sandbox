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
#include "System/public/filesystem.h"
#include "Vendor/lua/lua.hpp"
#include "Vendor/lua-rapidjson/rapidjson.cpp"


/////////////////////
// Namespace: Lua //
/////////////////////

namespace Vital::Lua {
    static const std::string vBlacklist[] = {
        "dofile",
        "load",
        "loadfile"
    };

    static const luaL_Reg Library_Whitelist[] = {
        {"_G", luaopen_base},
        {"table", luaopen_table},
        {"string", luaopen_string},
        {"math", luaopen_math},
        {"os", luaopen_os},
        {"coroutine", luaopen_coroutine},
        {"utf8", luaopen_utf8},
        {"json", luaopen_rapidjson},
        {NULL, NULL}
    };

    static const std::string vModules[] = {
        "namespacer.lua",
        "vcl.lua",
        "table.lua"
        "math/index.lua",
        "math/quat.lua",
        "math/matrix.lua",
        "string.lua",
        "thread.lua",
        "timer.lua"
    };
}