/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: private: engine.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Engine Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include "Sandbox/lua/public/index.h"


//////////////////////
// Lua: FileSystem //
//////////////////////

namespace Vital::Lua {
    bind("engine", "getSystemTick", [](vital_vm* vm) int {
        vm -> setInt(Vital::getSystemTick());
        return 1;
    });

    bind("engine", "getApplicationTick", [](vital_vm* vm) int {
        vm -> setInt(Vital::getSystemTick());
        return 1;
    });
}