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
#include <Sandbox/lua/public/index.h>
#include <Sandbox/lua/public/vm.h>


//////////////////////
// Lua: FileSystem //
//////////////////////

namespace Vital::Lua {
    void bind_Engine() {
        bind("engine", "getSystemTick", [](vital_vm* vm) -> int {
            vm -> setInt(Vital::getSystemTick());
            return 1;
        });

        bind("engine", "getApplicationTick", [](vital_vm* vm) -> int {
            vm -> setInt(Vital::getSystemTick());
            return 1;
        });
    }
}