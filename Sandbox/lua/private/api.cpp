/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: private: api.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: API Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/lua/public/api.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Lua::API {
    bool boot() {
        vSandbox_Engine();
        vSandbox_File();
    }

    bool bind(std::string parent, std::string name, std::function<int(vital_vm* vm)> exec) {
        vital_exec_ref ref = { parent, name };
        vMethods[ref] = [&](lua_State* vm) -> int {
            return exec(vInstances[vm]);
        };
        return true;
    }

    bool unbind(std::string parent, std::string name) {
        vital_exec_ref ref = { parent, name };
        vMethods.erase(ref);
        return true;
    }
}