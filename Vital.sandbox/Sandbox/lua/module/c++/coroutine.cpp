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
#include <Vendor/lua/lauxlib.h>
#include <Vendor/lua/lualib.h>


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

        bind("coroutine", "resume", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isThread(1))) throw ErrorCode["invalid-arguments"];
                auto thread = vm -> getThread(1);
                auto thread_vm = fetchVM(thread);
                //if (!isThread) throw throw ErrorCode["invalid-thread"];
                thread_vm -> resume();
                vm -> pushBool(true);
                return 1;
            });
        });

        bind("coroutine", "pause", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                //if (!isThread) throw throw ErrorCode["invalid-thread"];
                vm -> pause();
                return 1;
            });
        });

        bind("coroutine", "sleep", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw ErrorCode["invalid-arguments"];
                //if (!isThread) throw throw ErrorCode["invalid-thread"];
                auto duration = vm -> getInt(1);
                // TODO: Pause here
                std::cout << "\n v Thread 1 : " << std::this_thread::get_id();
                auto thread = Vital::System::Thread::create([&]() -> void {
                    // TODO: Resume here
                    std::cout << "\n v Thread 2 : " << std::this_thread::get_id();
                    std::cout << "going to sleep in vthread";
                    vm->pause();
                    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
                    std::cout << "back from sleep in vthread";
                    //vm -> resume();
                });
                return 1;
            });
        });
    }
}
