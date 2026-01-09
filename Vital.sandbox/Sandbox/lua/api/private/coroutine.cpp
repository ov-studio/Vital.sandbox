/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: private: coroutine.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Coroutine APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/lua/api/public/coroutine.h>
#include <Vital.sandbox/Vendor/lua/lauxlib.h>
#include <Vital.sandbox/Vendor/lua/lualib.h>


///////////////
// Lua: API //
///////////////

void Vital::Sandbox::Lua::API::Coroutine::bind(void* instance) {
    auto vm = static_cast<vsdk_vm*>(instance);

    API::bind(vm, "coroutine", "create", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isFunction(1))) throw Vital::Error::fetch("invalid-arguments");
            auto thread = vm -> createThread();
            vm -> push(1);
            vm -> move(thread, 1);
            return 1;
        });
    });

    API::bind(vm, "coroutine", "resume", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isThread(1))) throw Vital::Error::fetch("invalid-arguments");
            auto thread = vm -> getThread(1);
            auto thread_vm = fetchVM(thread);
            if (!thread_vm -> isVirtualThread()) throw Vital::Error::fetch("invalid-thread");
            thread_vm -> resume();
            vm -> setBool(true);
            return 1;
        });
    });

    API::bind(vm, "coroutine", "pause", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            if (!vm -> isVirtualThread()) throw Vital::Error::fetch("invalid-thread");
            vm -> pause();
            vm -> setBool(true);
            return 1;
        });
    });

    API::bind(vm, "coroutine", "sleep", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            if (!vm -> isVirtualThread()) throw Vital::Error::fetch("invalid-thread");
            if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw Vital::Error::fetch("invalid-arguments");
            auto duration = vm -> getInt(1);
            Vital::Type::Timer([=](Vital::Type::Timer* self) -> void {
                vm -> setBool(true);
                vm -> resume(1);
            }, duration, 1);
            vm -> pause();
            return 0;
        });
    });
}
