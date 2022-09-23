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
#include <Sandbox/lua/public/api.h>


/////////////////////////
// Lua: Engine Binder //
/////////////////////////

namespace Vital::Lua::API {
    void vSandbox_Engine() {
        bind("engine", "getSystemTick", [](vital_ref* vm) -> int {
            auto vm = fetchVM(vm);
            vm -> setInt(Vital::getSystemTick());
            return 1;
        });

        bind("engine", "getApplicationTick", [](vital_ref* vm) -> int {
            auto vm = fetchVM(vm);
            std::cout << "CALLED INNER";
            vm -> setInt(Vital::getApplicationTick());
            return 1;
        });
    }
}
