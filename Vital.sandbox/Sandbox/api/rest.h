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
    struct Rest : public vm_module {
        inline static const std::string base_name = "rest";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "get", [](auto vm) -> int {
                if (!vm -> is_virtual()) throw Vital::Log::fetch("invalid-thread", Vital::Log::Type::Error);
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto url = vm -> get_string(1);
                Vital::Tool::Rest::rest_headers headers = {};
                int timeout = 60;
                if (vm -> is_table(2)) {
                    for (int i = 1; i <= vm -> get_length(2); i++) {
                        vm -> get_table_field(i, 2);
                        if (vm -> is_string(-1)) headers.push_back(vm -> get_string(-1));
                        vm -> pop(1);
                    }
                }
                if (vm -> is_number(3)) timeout = vm -> get_int(3);
                Vital::Tool::Thread([=](Vital::Tool::Thread* thread) {
                    try {
                        vm -> push_value(Vital::Tool::Rest::get(url, headers, timeout));
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
            
            API::bind(vm, {base_name}, "post", [](auto vm) -> int {
                if (!vm -> is_virtual()) throw Vital::Log::fetch("invalid-thread", Vital::Log::Type::Error);
                if ((vm -> get_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto url = vm -> get_string(1);
                auto body = vm -> get_string(2);
                Vital::Tool::Rest::rest_headers headers = {};
                int timeout = 60;
                if (vm -> is_table(3)) {
                    for (int i = 1; i <= vm -> get_length(3); i++) {
                        vm -> get_table_field(i, 3);
                        if (vm -> is_string(-1)) headers.push_back(vm -> get_string(-1));
                        vm -> pop(1);
                    }
                }
                if (vm -> is_number(4)) timeout = vm -> get_int(4);
                Vital::Tool::Thread([=](Vital::Tool::Thread* thread) {
                    try {
                        vm -> push_value(Vital::Tool::Rest::post(url, body, headers, timeout));
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