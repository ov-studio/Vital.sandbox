/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: coroutine.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Coroutine APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>


////////////////////////////
// Vital: API: Coroutine //
////////////////////////////

namespace Vital::Sandbox::API {
    struct Coroutine : vm_module {
        inline static const std::string base_name = "coroutine";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(handler)")
                    .require(1, &Machine::is_function);

                auto thread = vm -> create_thread();
                vm -> push(1);
                vm -> move(thread, 1);
                return 1;
            });

            API::bind(vm, {base_name}, "resume", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(thread)")
                    .require(1, &Machine::is_thread);

                auto thread = vm -> get_thread(1);
                auto thread_vm = Machine::fetch_machine(thread);
                if (!thread_vm -> is_virtual()) throw Vital::Log::fetch("invalid-thread", Vital::Log::Type::Error);
                thread_vm -> resume();
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "pause", [](auto vm, auto& id) -> int {
                if (!vm -> is_virtual()) throw Vital::Log::fetch("invalid-thread", Vital::Log::Type::Error);
                vm -> pause();
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "sleep", [](auto vm, auto& id) -> int {
                if (!vm -> is_virtual()) throw Vital::Log::fetch("invalid-thread", Vital::Log::Type::Error);
                vm_args(vm, id, "(duration)")
                    .require(1, &Machine::is_number);

                auto duration = vm -> get_int(1);
                Tool::Timer([=](Tool::Timer* self) {
                    vm -> push_value(true);
                    vm -> resume(1);
                }, duration, 1);
                vm -> pause();
                return 0;
            });
        }
    };
}