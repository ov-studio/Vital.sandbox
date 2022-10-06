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

namespace Vital::Sandbox::Lua::API {
    void vSandbox_Engine() {
        bind("engine", "getSystemTick", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([]() -> int {
                vm -> setNumber(static_cast<int>(Vital::System::getSystemTick()));
                return 1;
            });
        });

        bind("engine", "getApplicationTick", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([]() -> int {
                vm -> setNumber(static_cast<int>(Vital::System::getApplicationTick()));
                return 1;
            });
        });

        bind("engine", "getClientTick", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([]() -> int {
                vm -> setNumber(static_cast<int>(Vital::System::getClientTick()));
                return 1;
            });
        });
    }
}
