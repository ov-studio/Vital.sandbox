/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: crypto.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Crypto APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>


/////////////////////////
// Vital: API: Crypto //
/////////////////////////

namespace Vital::Sandbox::API {
    struct Crypto : vm_module {
        inline static const std::string base_name = "crypto";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "hash", [](auto vm) -> int {
                vm_args(vm, "crypto.hash(mode, input)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string);
                    
                auto mode = vm -> get_string(1);
                auto input = vm -> get_string(2);
                vm -> push_value(Tool::Crypto::hash(mode, input));
                return 1;
            });

            API::bind(vm, {base_name}, "encode", [](auto vm) -> int {
                vm_args(vm, "crypto.encode(input)")
                    .require(1, &Machine::is_string);

                auto input = vm -> get_string(1);
                vm -> push_value(Tool::Crypto::encode(input));
                return 1;
            });

            API::bind(vm, {base_name}, "decode", [](auto vm) -> int {
                vm_args(vm, "crypto.decode(input)")
                    .require(1, &Machine::is_string);

                auto input = vm -> get_string(1);
                vm -> push_value(Tool::Crypto::decode(input));
                return 1;
            });

            API::bind(vm, {base_name}, "encrypt", [](auto vm) -> int {
                vm_args(vm, "crypto.encrypt(mode, input, key)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string);

                auto mode = vm -> get_string(1);
                auto input = vm -> get_string(2);
                auto key = vm -> get_string(3);
                auto result = Tool::Crypto::encrypt(mode, input, key);
                vm -> push_value(result.first);
                vm -> push_value(result.second);
                return 2;
            });

            API::bind(vm, {base_name}, "decrypt", [](auto vm) -> int {
                vm_args(vm, "crypto.decrypt(mode, input, key, iv)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_string);

                auto mode = vm -> get_string(1);
                auto input = vm -> get_string(2);
                auto key = vm -> get_string(3);
                auto iv = vm -> get_string(4);
                vm -> push_value(Tool::Crypto::decrypt(mode, input, key, iv));
                return 1;
            });
        }
    };
}