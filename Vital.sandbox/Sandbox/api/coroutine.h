/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: coroutine.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Coroutine APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/index.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::API {
    class Coroutine : public Vital::Tool::Module {
        public:
            inline static void bind(void* instance) {
                auto vm = Vital::Sandbox::Lua::to_vm(instance);

                API::bind(vm, "coroutine", "create", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_function(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto thread = vm -> create_thread();
                        vm -> push(1);
                        vm -> move(thread, 1);
                        return 1;
                    });
                });
            
                API::bind(vm, "coroutine", "resume", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_thread(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto thread = vm -> get_thread(1);
                        auto thread_vm = Vital::Sandbox::Lua::fetch_vm(thread);
                        if (!thread_vm -> is_virtual()) throw Vital::Error::fetch("invalid-thread");
                        thread_vm -> resume();
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                API::bind(vm, "coroutine", "pause", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if (!vm -> is_virtual()) throw Vital::Error::fetch("invalid-thread");
                        vm -> pause();
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            
                API::bind(vm, "coroutine", "sleep", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if (!vm -> is_virtual()) throw Vital::Error::fetch("invalid-thread");
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto duration = vm -> get_int(1);
                        Vital::Tool::Timer([=](Vital::Tool::Timer* self) -> void {
                            vm -> push_bool(true);
                            vm -> resume(1);
                        }, duration, 1);
                        vm -> pause();
                        return 0;
                    });
                });
            }
    };
}