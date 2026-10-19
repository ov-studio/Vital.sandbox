/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: http.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: HTTP APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/API/utility/promise.h>
#include <Vital.sandbox/API/utility/file.h>


///////////////////////
// Vital: API: HTTP //
///////////////////////

namespace Vital::Sandbox::API {
    struct HTTP : public vm_module {
        inline static const std::vector<std::string> base_scope = {"util", "http"};
        static constexpr size_t max_download_size = 1024ull * 1024 * 1024;

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "get", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(url, headers = {\"Content-Type: application/json\"}, timeout = 60)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_table)
                    .optional(3, &Machine::is_number);

                auto url = vm -> get_string(1);
                Tool::HTTP::http_headers headers = {};
                int timeout = 60;
                if (vm -> is_table(2)) {
                    for (int i = 1; i <= vm -> get_length(2); i++) {
                        vm -> table_get_value(i, 2);
                        if (vm -> is_string(-1)) headers.push_back(vm -> get_string(-1));
                        vm -> pop(1);
                    }
                }
                if (vm -> is_number(3)) timeout = vm -> get_int(3);

                auto slot = std::make_shared<Tool::HTTP::Slot>();
                auto promise_id = API::Promise::make(vm, true) -> id;
                Tool::Thread::create([promise_id, url, headers, timeout, slot](Tool::Thread*) {
                    try {
                        auto response = Tool::HTTP::get(url, headers, timeout);
                        Machine::enqueue([promise_id, response]() {
                            auto promise = API::Promise::Instance::find(promise_id);
                            if (!API::Promise::is_pending(promise)) return;
                            auto vm = promise -> vm;
                            vm -> push_value(response);
                            API::Promise::settle(promise, API::Promise::State::Resolved, vm, vm -> get_count(), 1);
                        });
                    }
                    catch (const std::runtime_error& error) {
                        std::string message = error.what();
                        Machine::enqueue([promise_id, message]() {
                            auto promise = API::Promise::Instance::find(promise_id);
                            if (!API::Promise::is_pending(promise)) return;
                            auto vm = promise -> vm;
                            vm -> push_value(message);
                            API::Promise::settle(promise, API::Promise::State::Rejected, vm, vm -> get_count(), 1);
                        });
                    }
                }) -> detach();
                return 1;
            });

            API::bind(vm, base_scope, "download", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(url, path, headers = {\"Content-Type: application/json\"}, timeout = 60)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string)
                    .optional(3, &Machine::is_table)
                    .optional(4, &Machine::is_number);

                auto url = vm -> get_string(1);
                auto path = vm -> get_string(2);
                const std::string base = API::File::get_base(vm, path, true);
                const std::filesystem::path dest = std::filesystem::path(base) / path;
                Tool::HTTP::http_headers headers = {};
                int timeout = 60;
                if (vm -> is_table(3)) {
                    for (int i = 1; i <= vm -> get_length(3); i++) {
                        vm -> table_get_value(i, 3);
                        if (vm -> is_string(-1)) headers.push_back(vm -> get_string(-1));
                        vm -> pop(1);
                    }
                }
                if (vm -> is_number(4)) timeout = vm -> get_int(4);

                auto slot = std::make_shared<Tool::HTTP::Slot>();
                auto promise_id = API::Promise::make(vm, true) -> id;
                Tool::Thread::create([promise_id, url, dest, headers, timeout, slot](Tool::Thread*) {
                    auto settle = [promise_id](API::Promise::State state, auto value) {
                        Machine::enqueue([promise_id, state, value]() {
                            auto promise = API::Promise::Instance::find(promise_id);
                            if (!API::Promise::is_pending(promise)) return;
                            auto vm = promise -> vm;
                            vm -> push_value(value);
                            API::Promise::settle(promise, state, vm, vm -> get_count(), 1);
                        });
                    };
                    std::filesystem::path part = dest;
                    part += ".part";
                    try {
                        Tool::HTTP::download(url, part.string(), max_download_size, headers, timeout);
                        std::filesystem::rename(part, dest);
                        settle(API::Promise::State::Resolved, true);
                    }
                    catch (const std::exception& error) {
                        std::error_code ec;
                        std::filesystem::remove(part, ec);
                        settle(API::Promise::State::Rejected, std::string(error.what()));
                    }
                }) -> detach();
                return 1;
            });

            API::bind(vm, base_scope, "post", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(url, body, headers = {\"Content-Type: application/json\"}, timeout = 60)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string)
                    .optional(3, &Machine::is_table)
                    .optional(4, &Machine::is_number);

                auto url = vm -> get_string(1);
                auto body = vm -> get_string(2);
                Tool::HTTP::http_headers headers = {};
                int timeout = 60;
                if (vm -> is_table(3)) {
                    for (int i = 1; i <= vm -> get_length(3); i++) {
                        vm -> table_get_value(i, 3);
                        if (vm -> is_string(-1)) headers.push_back(vm -> get_string(-1));
                        vm -> pop(1);
                    }
                }
                if (vm -> is_number(4)) timeout = vm -> get_int(4);

                auto slot = std::make_shared<Tool::HTTP::Slot>();
                auto promise_id = API::Promise::make(vm, true) -> id;
                Tool::Thread::create([promise_id, url, body, headers, timeout, slot](Tool::Thread*) {
                    try {
                        auto response = Tool::HTTP::post(url, body, headers, timeout);
                        Machine::enqueue([promise_id, response]() {
                            auto promise = API::Promise::Instance::find(promise_id);
                            if (!API::Promise::is_pending(promise)) return;
                            auto vm = promise -> vm;
                            vm -> push_value(response);
                            API::Promise::settle(promise, API::Promise::State::Resolved, vm, vm -> get_count(), 1);
                        });
                    }
                    catch (const std::runtime_error& error) {
                        std::string message = error.what();
                        Machine::enqueue([promise_id, message]() {
                            auto promise = API::Promise::Instance::find(promise_id);
                            if (!API::Promise::is_pending(promise)) return;
                            auto vm = promise -> vm;
                            vm -> push_value(message);
                            API::Promise::settle(promise, API::Promise::State::Rejected, vm, vm -> get_count(), 1);
                        });
                    }
                }) -> detach();
                return 1;
            });
        }
    };
}