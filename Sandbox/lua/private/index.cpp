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
#include <Sandbox/lua/public/api.h>


/////////////////////
// Namespace: Lua //
/////////////////////

namespace Vital::Sandbox::Lua {
    std::vector<std::string> vBlacklist = {
        "dofile",
        "load",
        "loadfile"
    };

    std::vector<luaL_Reg> vLibrary = {
        {"_G", luaopen_base},
        {"table", luaopen_table},
        {"string", luaopen_string},
        {"math", luaopen_math},
        {"os", luaopen_os},
        {"debug", luaopen_debug},
        {"coroutine", luaopen_coroutine},
        {"utf8", luaopen_utf8}
    };

    std::vector<std::string> vModules = {
        "namespace.lua",
        "vcl.lua",
        "table.lua",
        "math/index.lua",
        "math/quat.lua",
        "math/matrix.lua",
        "string.lua",
        "thread.lua",
        "timer.lua",
        "engine.lua",
        "crypto.lua",
        "file.lua",
        "resource.lua"
    };
}