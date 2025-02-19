/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: private: engine.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Engine APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/lua/api/public/engine.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    void Engine::bind(void* instance) {
        API::bind("engine", "getSystemTick", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                vm -> setNumber(static_cast<int>(Vital::System::getSystemTick()));
                return 1;
            });
        });

        API::bind("engine", "getAppTick", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                vm -> setNumber(static_cast<int>(Vital::System::getAppTick()));
                return 1;
            });
        });

        API::bind("engine", "getClientTick", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                vm -> setNumber(static_cast<int>(Vital::System::getClientTick()));
                return 1;
            });
        });

        API::bind("engine", "loadString", [](auto* ref) -> int {
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