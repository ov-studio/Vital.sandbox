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

        //std::map<std::thread, bool> buffer;
        bind("coroutine", "sleep", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw ErrorCode["invalid-arguments"];
                auto duration = vm -> getInt(1);
                std::cout << "PREV THREAD : " << std::this_thread::get_id();
                auto thread = Vital::System::Thread::create([&]() -> void {
                    std::cout << "\nCURR THREAD : " << std::this_thread::get_id();
                    std::cout << "\nSLEEPING FOR DURATION: " << duration;
                    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
                    std::cout << "\nWOKE UP : " << std::this_thread::get_id();
                });
                std::cout << "\n??? : " << std::this_thread::get_id();
                return 0;
            });
        });
    }
}
