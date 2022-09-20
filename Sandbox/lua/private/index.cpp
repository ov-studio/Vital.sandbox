/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: public: index.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <string>
#include <Sandbox/lua/public/index.h>
#include "Vendor/lua-rapidjson/rapidjson.cpp"


/////////////////////
// Namespace: Lua //
/////////////////////

namespace Vital::Lua {
    std::string vBlacklist[] = {
        "dofile",
        "load",
        "loadfile"
    };

    luaL_Reg Library_Whitelist[] = {
        {"_G", luaopen_base},
        {"table", luaopen_table},
        {"string", luaopen_string},
        {"math", luaopen_math},
        {"os", luaopen_os},
        {"debug", luaopen_debug},
        {"coroutine", luaopen_coroutine},
        {"utf8", luaopen_utf8},
        {"json", luaopen_rapidjson},
        {NULL, NULL}
    };

    std::string vModules[] = {
        "namespace.lua",
        "vcl.lua",
        "table.lua"
        "math/index.lua",
        "math/quat.lua",
        "math/matrix.lua",
        "string.lua",
        "thread.lua",
        "timer.lua",
        "resource.lua"
    };
}