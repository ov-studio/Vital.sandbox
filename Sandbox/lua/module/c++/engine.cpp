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
            try {
                vm -> setNumber(static_cast<int>(Vital::System::getSystemTick()));
            }
            catch(const std::string& error) { vm -> throwError(error); }
            catch(...) { vm -> throwError(); }
            return 1;
        });

        bind("engine", "getApplicationTick", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            try {
                vm -> setNumber(static_cast<int>(Vital::System::getApplicationTick()));
            }
            catch(const std::string& error) { vm -> throwError(error); }
            catch(...) { vm -> throwError(); }
            return 1;
        });

        bind("engine", "getClientTick", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            try {
                vm -> setNumber(static_cast<int>(Vital::System::getClientTick()));
            }
            catch(const std::string& error) { vm -> throwError(error); }
            catch(...) { vm -> throwError(); }
            return 1;
        });
    }
}
