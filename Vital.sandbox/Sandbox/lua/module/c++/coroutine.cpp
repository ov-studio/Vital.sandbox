/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: module: c++: coroutine.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Coroutine Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/thread.h>
#include <Sandbox/lua/public/api.h>


////////////////////////////
// Lua: Coroutine Binder //
////////////////////////////

namespace Vital::Sandbox::Lua::API {
    static bool isBound = false;
    void vSandbox_Coroutine() {
        if (isBound) return; isBound = true;

        bind("coroutine", "create", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isFunction(1))) throw ErrorCode["invalid-arguments"];
                auto thread = vm -> createThread();
                vm -> push(1);
                vm -> move(thread, 1);
                return 1;
            });
        });

        bind("coroutine", "customresume", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isThread(1))) throw ErrorCode["invalid-arguments"];
                auto thread = vm -> getThread(1);
                std::cout << "\nCustom Resume Invoked";
                lua_resume(thread, thread, 0, 0);
                return 1;
            });
        });

        bind("coroutine", "pause", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isThread(1))) throw ErrorCode["invalid-arguments"];
                lua_yield(ref, 0);
                return 1;
            });
        });

        bind("coroutine", "sleep", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw ErrorCode["invalid-arguments"];
                auto duration = vm -> getInt(1);
                auto thread = Vital::System::Thread::create([&]() -> void {
                    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
                });
                return 1;
            });
        });
    }
}
