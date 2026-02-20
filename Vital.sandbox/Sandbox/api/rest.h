/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: rest.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rest APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>


////////////////////////////////
// Vital: Sandbox: API: Rest //
////////////////////////////////

namespace Vital::Sandbox::API {
    struct Rest : public Vital::Tool::Module {
        static void bind(void* machine) {
            auto vm = Machine::to_machine(machine);

            API::bind(vm, "rest", "get", [](auto* vm) -> int {
                if (!vm -> is_virtual()) throw Vital::Log::fetch("invalid-thread", Vital::Log::Type::Error);
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto url = vm -> get_string(1);
                Vital::Tool::Rest::rest_headers headers = {};
                if (vm -> is_table(2)) {
                    for (int i = 1; i <= vm -> get_length(2); i++) {
                        vm -> get_table_field(i, 2);
                        if (vm -> is_string(-1)) headers.push_back(vm -> get_string(-1));
                        vm -> pop(1);
                    }
                }
                Vital::Tool::Thread([=](Vital::Tool::Thread* thread) -> void {
                    try {
                        vm -> push_string(headers.empty() ? Vital::Tool::Rest::get(url) : Vital::Tool::Rest::get(url, headers));
                        vm -> push_bool(false);
                    }
                    catch(const std::runtime_error& error) {
                        vm -> push_bool(false);
                        vm -> push_string(error.what());
                    }
                    vm -> resume(2);
                }).detach();
                vm -> pause();
                return 0;
            });

            API::bind(vm, "rest", "post", [](auto* vm) -> int {
                if (!vm -> is_virtual()) throw Vital::Log::fetch("invalid-thread", Vital::Log::Type::Error);
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto url = vm -> get_string(1);
                auto body = vm -> get_string(2);
                Vital::Tool::Rest::rest_headers headers = {};
                if (vm -> is_table(3)) {
                    for (int i = 1; i <= vm -> get_length(3); i++) {
                        vm -> get_table_field(i, 3);
                        if (vm -> is_string(-1)) headers.push_back(vm -> get_string(-1));
                        vm -> pop(1);
                    }
                }
                Vital::Tool::Thread([=](Vital::Tool::Thread* thread) -> void {
                    try {
                        vm -> push_string(headers.empty() ? Vital::Tool::Rest::post(url, body) : Vital::Tool::Rest::post(url, body, headers));
                        vm -> push_bool(false);
                    }
                    catch(const std::runtime_error& error) {
                        vm -> push_bool(false);
                        vm -> push_string(error.what());
                    }
                    vm -> resume(2);
                }).detach();
                vm -> pause();
                return 0;
            });
        }
    };
}