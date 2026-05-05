/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: rest.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rest APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/API/utility/promise.h>


///////////////////////
// Vital: API: Rest //
///////////////////////

namespace Vital::Sandbox::API {
    struct Rest : public vm_module {
        inline static const std::string base_name = "rest";

        // Creates a promise, registers it, pushes it onto the stack, returns it.
        // The lambda then captures the promise id and settles it from a background thread.
        // settle() pushes values directly onto instance->vm (the shared lua_State) — no scratch machine.
        static std::shared_ptr<Promise::Instance> make_promise(Machine* vm) {
            std::string env = vm->get_environment_id();
            auto instance = std::make_shared<Promise::Instance>();
            instance->id  = Promise::next_id.fetch_add(1);
            instance->env = env;
            instance->vm  = vm;
            {
                std::lock_guard<std::mutex> lock(Promise::mutex);
                Promise::buffer[instance->id] = instance;
            }
            vm->create_object(Promise::base_name, instance.get());
            instance->userdata = vm_module::get_userdata_ptr(vm, -1);
            return instance;
        }

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "get", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(url, headers = {\"Content-Type: application/json\"}, timeout = 60)")
                    .require(1, &Machine::is_string);

                auto url = vm->get_string(1);
                Tool::Rest::rest_headers headers = {};
                int timeout = 60;
                if (vm->is_table(2)) {
                    for (int i = 1; i <= vm->get_length(2); i++) {
                        vm->get_table_field(i, 2);
                        if (vm->is_string(-1)) headers.push_back(vm->get_string(-1));
                        vm->pop(1);
                    }
                }
                if (vm->is_number(3)) timeout = vm->get_int(3);

                auto instance = make_promise(vm);
                int promise_id = instance->id;

                Tool::Thread::create([promise_id, url, headers, timeout](Tool::Thread*) {
                    auto inst = Promise::fetch_instance(promise_id);
                    if (!inst || inst->destroyed) return;
                    Machine* vm = inst->vm;
                    try {
                        vm->push_value(Tool::Rest::get(url, headers, timeout));
                        // stack: [..., result]  args_start = top, args_count = 1
                        Promise::settle(inst, Promise::State::Resolved, vm, vm->get_count(), 1);
                        vm->pop(1);
                    } catch (const std::runtime_error& error) {
                        vm->push_value(std::string(error.what()));
                        Promise::settle(inst, Promise::State::Rejected, vm, vm->get_count(), 1);
                        vm->pop(1);
                    }
                })->detach();
                return 1;
            });

            API::bind(vm, {base_name}, "post", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(url, body, headers = {\"Content-Type: application/json\"}, timeout = 60)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string);

                auto url  = vm->get_string(1);
                auto body = vm->get_string(2);
                Tool::Rest::rest_headers headers = {};
                int timeout = 60;
                if (vm->is_table(3)) {
                    for (int i = 1; i <= vm->get_length(3); i++) {
                        vm->get_table_field(i, 3);
                        if (vm->is_string(-1)) headers.push_back(vm->get_string(-1));
                        vm->pop(1);
                    }
                }
                if (vm->is_number(4)) timeout = vm->get_int(4);

                auto instance = make_promise(vm);
                int promise_id = instance->id;

                Tool::Thread::create([promise_id, url, body, headers, timeout](Tool::Thread*) {
                    auto inst = Promise::fetch_instance(promise_id);
                    if (!inst || inst->destroyed) return;
                    Machine* vm = inst->vm;
                    try {
                        vm->push_value(Tool::Rest::post(url, body, headers, timeout));
                        Promise::settle(inst, Promise::State::Resolved, vm, vm->get_count(), 1);
                        vm->pop(1);
                    } catch (const std::runtime_error& error) {
                        vm->push_value(std::string(error.what()));
                        Promise::settle(inst, Promise::State::Rejected, vm, vm->get_count(), 1);
                        vm->pop(1);
                    }
                })->detach();
                return 1;
            });
        }
    };
}