/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: module: c++: engine.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
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
    static bool isBound = false;
    void vSandbox_Engine() {
        if (isBound) return; isBound = true;

        bind("engine", "getSystemTick", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                vm -> setNumber(static_cast<int>(Vital::System::getSystemTick()));
                return 1;
            });
        });

        bind("engine", "getApplicationTick", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                vm -> setNumber(static_cast<int>(Vital::System::getApplicationTick()));
                return 1;
            });
        });

        bind("engine", "getClientTick", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                vm -> setNumber(static_cast<int>(Vital::System::getClientTick()));
                return 1;
            });
        });

        bind("engine", "loadString", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw ErrorCode["invalid-arguments"];
                std::string rwString = vm -> getString(1);
                bool result = false;
                if (vm -> isBool(2)) {
                    bool isAutoLoad = vm -> getBool(2);
                    result = vm -> loadString(rwString, isAutoLoad);
                    if (result) return 1;
                }
                else result = vm -> loadString(rwString);
                vm -> setBool(result);
                return 1;
            });
        });
    }
}
