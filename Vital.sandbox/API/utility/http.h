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

        static Tool::HTTP::http_headers read_headers(Machine* vm, int idx) {
            Tool::HTTP::http_headers headers = {};
            if (vm -> is_table(idx)) {
                for (int i = 1; i <= vm -> get_length(idx); i++) {
                    vm -> table_get_value(i, idx);
                    if (vm -> is_string(-1)) headers.push_back(vm -> get_string(-1));
                    vm -> pop(1);
                }
            }
            return headers;
        }

        static int read_timeout(Machine* vm, int idx) {
            return vm -> is_number(idx) ? vm -> get_int(idx) : 60;
        }

        template<typename Id, typename T>
        static void settle(Id promise_id, API::Promise::State state, T value) {
            Machine::enqueue([promise_id, state, value]() {
                auto promise = API::Promise::Instance::find(promise_id);
                if (!API::Promise::is_pending(promise)) return;
                auto vm = promise -> vm;
                vm -> push_value(value);
                API::Promise::settle(promise, state, vm, vm -> get_count(), 1);
            });
        }

        template<typename Work>
        static void request(Machine* vm, Work work) {
            auto slot = std::make_shared<Tool::HTTP::Slot>();
            auto promise_id = API::Promise::make(vm, true) -> id;
            Tool::Thread::create([promise_id, work, slot](Tool::Thread*) {
                try { settle(promise_id, API::Promise::State::Resolved, work()); }
                catch (const std::exception& error) { settle(promise_id, API::Promise::State::Rejected, std::string(error.what())); }
            }) -> detach();
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "get", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(url, headers = {\"Content-Type: application/json\"}, timeout = 60)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_table)
                    .optional(3, &Machine::is_number);

                auto url = vm -> get_string(1);
                auto headers = read_headers(vm, 2);
                auto timeout = read_timeout(vm, 3);
                request(vm, [url, headers, timeout]() { return Tool::HTTP::get(url, headers, timeout); });
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
                auto headers = read_headers(vm, 3);
                auto timeout = read_timeout(vm, 4);
                request(vm, [url, dest, headers, timeout]() {
                    std::filesystem::path part = dest;
                    part += ".part";
                    try {
                        Tool::HTTP::download(url, part.string(), Tool::HTTP::DEFAULT_MAX_DOWNLOAD, headers, timeout);
                        std::filesystem::rename(part, dest);
                    }
                    catch (...) {
                        std::error_code ec;
                        std::filesystem::remove(part, ec);
                        throw;
                    }
                    return true;
                });
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
                auto headers = read_headers(vm, 3);
                auto timeout = read_timeout(vm, 4);
                request(vm, [url, body, headers, timeout]() { return Tool::HTTP::post(url, body, headers, timeout); });
                return 1;
            });
        }
    };
}
