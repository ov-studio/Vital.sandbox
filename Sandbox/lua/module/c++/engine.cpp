/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: module: c++: engine.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Engine Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>
#include <Sandbox/lua/public/index.h>
#include <Sandbox/lua/public/vm.h>


/////////////////////////
// Lua: Engine Binder //
/////////////////////////

namespace Vital::Lua {
    void vSandbox_Engine_API() {
        bind("engine", "getSystemTick", [](vital_vm* vm) -> int {
            vm -> setInt(Vital::getSystemTick());
            return 1;
        });

        bind("engine", "getApplicationTick", [](vital_vm* vm) -> int {
            vm -> setInt(Vital::getApplicationTick());
            return 1;
        });
    }
}