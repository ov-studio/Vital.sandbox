/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: public: index.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/lua/public/api.h>


/////////////////////
// Namespace: Lua //
/////////////////////

namespace Vital::Sandbox::Lua {
    std::vector<luaL_Reg> vsdk_libraries = {
        {"_G", luaopen_base},
        {"table", luaopen_table},
        {"string", luaopen_string},
        {"math", luaopen_math},
        {"debug", luaopen_debug},
        {"coroutine", luaopen_coroutine},
        {"utf8", luaopen_utf8},
        {"json", luaopen_rapidjson}
    };

    std::vector<std::string> vsdk_modules = {
        "namespace.lua",
        "vcl.lua",
        "table.lua",
        "math/index.lua",
        "math/quat.lua",
        "math/matrix.lua",
        "string.lua",
        "threader.lua",
        "timer.lua",
        "engine.lua",
        "file.lua",
        "crypto.lua",
        "network.lua",
        "rest.lua",
        "resource.lua"
    };

    std::vector<std::string> vsdk_blacklist = {
        "dofile",
        "load",
        "loadfile"
    };
}