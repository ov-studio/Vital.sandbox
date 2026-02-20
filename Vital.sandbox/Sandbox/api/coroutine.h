/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: coroutine.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Coroutine APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>


/////////////////////////////////////
// Vital: Sandbox: API: Coroutine //
/////////////////////////////////////

namespace Vital::Sandbox::API {
    struct Coroutine : Vital::Tool::Module {
        static void bind(void* machine) {
            auto vm = Machine::to_machine(machine);

            API::bind(vm, "coroutine", "create", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_function(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto thread = vm -> create_thread();
                vm -> push(1);
                vm -> move(thread, 1);
                return 1;
            });
        
            API::bind(vm, "coroutine", "resume", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_thread(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto thread = vm -> get_thread(1);
                auto thread_vm = Machine::fetch_machine(thread);
                if (!thread_vm -> is_virtual()) throw Vital::Log::fetch("invalid-thread", Vital::Log::Type::Error);
                thread_vm -> resume();
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, "coroutine", "pause", [](auto* vm) -> int {
                if (!vm -> is_virtual()) throw Vital::Log::fetch("invalid-thread", Vital::Log::Type::Error);
                vm -> pause();
                vm -> push_bool(true);
                return 1;
            });
        
            API::bind(vm, "coroutine", "sleep", [](auto* vm) -> int {
                if (!vm -> is_virtual()) throw Vital::Log::fetch("invalid-thread", Vital::Log::Type::Error);
                if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto duration = vm -> get_int(1);
                Vital::Tool::Timer([=](Vital::Tool::Timer* self) -> void {
                    vm -> push_bool(true);
                    vm -> resume(1);
                }, duration, 1);
                vm -> pause();
                return 0;
            });
        }
    };
}