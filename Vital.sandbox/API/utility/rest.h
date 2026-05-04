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

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "get", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(url, headers = {\"Content-Type: application/json\"}, timeout = 60)")
                    .require(1, &Machine::is_string);

                auto url = vm -> get_string(1);
                Tool::Rest::rest_headers headers = {};
                int timeout = 60;
                if (vm -> is_table(2)) {
                    for (int i = 1; i <= vm -> get_length(2); i++) {
                        vm -> get_table_field(i, 2);
                        if (vm -> is_string(-1)) headers.push_back(vm -> get_string(-1));
                        vm -> pop(1);
                    }
                }
                if (vm -> is_number(3)) timeout = vm -> get_int(3);

                // Create a promise and return it to Lua — caller is responsible for threading/await.
                std::string env = vm -> get_environment_id();
                auto instance = std::make_shared<Promise::Instance>();
                instance -> id = Promise::next_id.fetch_add(1);
                instance -> env = env;
                instance -> vm = vm;
                {
                    std::lock_guard<std::mutex> lock(Promise::mutex);
                    Promise::buffer[instance -> id] = instance;
                }
                vm -> create_object(Promise::base_name, instance.get());
                instance -> userdata = vm_module::get_userdata_ptr(vm, -1);
                // Keep the promise on the stack (return value).

                int promise_id = instance -> id;
                Tool::Thread::create([promise_id, url, headers, timeout](Tool::Thread*) {
                    auto inst = Promise::fetch_instance(promise_id);
                    if (!inst || inst -> destroyed) return;
                    // TODO: SKIP SERIALIZATION COMPLETELY
                    // We need a temporary scratch vm_state* to push values through settle().
                    // settle() serialises immediately, so we create a transient Machine just for that.
                    Machine scratch;
                    try {
                        scratch.push_value(Tool::Rest::get(url, headers, timeout));
                        scratch.push_value(false);
                        Promise::settle(inst, Promise::State::Resolved, &scratch, 1, 2);
                    }
                    catch (const std::runtime_error& error) {
                        scratch.push_value(false);
                        scratch.push_value(error.what());
                        Promise::settle(inst, Promise::State::Rejected, &scratch, 1, 2);
                    }
                }) -> detach();
                return 1;
            });

            API::bind(vm, {base_name}, "post", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(url, body, headers = {\"Content-Type: application/json\"}, timeout = 60)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string);

                auto url = vm -> get_string(1);
                auto body = vm -> get_string(2);
                Tool::Rest::rest_headers headers = {};
                int timeout = 60;
                if (vm -> is_table(3)) {
                    for (int i = 1; i <= vm -> get_length(3); i++) {
                        vm -> get_table_field(i, 3);
                        if (vm -> is_string(-1)) headers.push_back(vm -> get_string(-1));
                        vm -> pop(1);
                    }
                }
                if (vm -> is_number(4)) timeout = vm -> get_int(4);

                std::string env = vm -> get_environment_id();
                auto instance = std::make_shared<Promise::Instance>();
                instance -> id = Promise::next_id.fetch_add(1);
                instance -> env = env;
                instance -> vm = vm;
                {
                    std::lock_guard<std::mutex> lock(Promise::mutex);
                    Promise::buffer[instance -> id] = instance;
                }
                vm -> create_object(Promise::base_name, instance.get());
                instance -> userdata = vm_module::get_userdata_ptr(vm, -1);

                int promise_id = instance -> id;
                Tool::Thread::create([promise_id, url, body, headers, timeout](Tool::Thread*) {
                    auto inst = Promise::fetch_instance(promise_id);
                    if (!inst || inst -> destroyed) return;
                    // TODO: SKIP SERIALIZATION COMPLETELY
                    Machine scratch;
                    try {
                        scratch.push_value(Tool::Rest::post(url, body, headers, timeout));
                        scratch.push_value(false);
                        Promise::settle(inst, Promise::State::Resolved, &scratch, 1, 2);
                    }
                    catch (const std::runtime_error& error) {
                        scratch.push_value(false);
                        scratch.push_value(error.what());
                        Promise::settle(inst, Promise::State::Rejected, &scratch, 1, 2);
                    }
                }) -> detach();
                return 1;
            });
        }
    };
}