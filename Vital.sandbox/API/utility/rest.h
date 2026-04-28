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


///////////////////////
// Vital: API: Rest //
///////////////////////

namespace Vital::Sandbox::API {
    struct Rest : public vm_module {
        inline static const std::string base_name = "rest";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "get", [](auto vm, auto& id) -> int {
                if (!vm -> is_virtual()) throw Tool::Log::fetch("invalid-thread", Tool::Log::Type::error);
                vm_args(vm, id, "(url, headers, timeout)")
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
                Tool::Thread([=](Tool::Thread* thread) {
                    try {
                        vm -> push_value(Tool::Rest::get(url, headers, timeout));
                        vm -> push_value(false);
                    }
                    catch(const std::runtime_error& error) {
                        vm -> push_value(false);
                        vm -> push_value(error.what());
                    }
                    vm -> resume(2);
                }).detach();
                vm -> pause();
                return 0;
            });

            API::bind(vm, {base_name}, "post", [](auto vm, auto& id) -> int {
                if (!vm -> is_virtual()) throw Tool::Log::fetch("invalid-thread", Tool::Log::Type::error);
                vm_args(vm, id, "(url, body, headers, timeout)")
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
                Tool::Thread([=](Tool::Thread* thread) {
                    try {
                        vm -> push_value(Tool::Rest::post(url, body, headers, timeout));
                        vm -> push_value(false);
                    }
                    catch(const std::runtime_error& error) {
                        vm -> push_value(false);
                        vm -> push_value(error.what());
                    }
                    vm -> resume(2);
                }).detach();
                vm -> pause();
                return 0;
            });
        }
    };
}