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
#include <Sandbox/lua/api/public/coroutine.h>
#include <Vendor/lua/lauxlib.h>
#include <Vendor/lua/lualib.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    bool Coroutine::bound = false;

    void Coroutine::boot() {
        if (bound) return;
        bound = true;

        bind("coroutine", "create", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isFunction(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto thread = vm -> createThread();
                vm -> push(1);
                vm -> move(thread, 1);
                return 1;
            });
        });

        bind("coroutine", "resume", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isThread(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto thread = vm -> getThread(1);
                auto thread_vm = fetchVM(thread);
                if (!thread_vm -> isVirtualThread()) throw std::runtime_error(ErrorCode["invalid-thread"]);
                thread_vm -> resume();
                vm -> setBool(true);
                return 1;
            });
        });

        bind("coroutine", "pause", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if (!vm -> isVirtualThread()) throw std::runtime_error(ErrorCode["invalid-thread"]);
                vm -> pause();
                vm -> setBool(true);
                return 1;
            });
        });

        bind("coroutine", "sleep", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if (!vm -> isVirtualThread()) throw std::runtime_error(ErrorCode["invalid-thread"]);
                if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto duration = vm -> getInt(1);
                Vital::Type::Timer([=](Vital::Type::Timer* self) -> void {
                    vm -> resume();
                }, duration, 1);
                vm -> pause();
                vm -> setBool(true);
                return 1;
            });
        });
    }
}