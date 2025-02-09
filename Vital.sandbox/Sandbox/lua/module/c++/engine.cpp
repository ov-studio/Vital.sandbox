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

        bind("engine", "getSystemTick", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                vm -> setNumber(static_cast<int>(Vital::System::getSystemTick()));
                return 1;
            });
        });

        bind("engine", "getAppTick", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                vm -> setNumber(static_cast<int>(Vital::System::getAppTick()));
                return 1;
            });
        });

        bind("engine", "getClientTick", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                vm -> setNumber(static_cast<int>(Vital::System::getClientTick()));
                return 1;
            });
        });

        bind("engine", "loadString", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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
