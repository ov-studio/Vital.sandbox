/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: crypto.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Crypto APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>


//////////////////////////////////
// Vital: Sandbox: API: Crypto //
//////////////////////////////////

namespace Vital::Sandbox::API {
    struct Crypto : Module {
        static void bind(Machine* vm) {
            API::bind(vm, "crypto", "hash", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto mode = vm -> get_string(1);
                auto input = vm -> get_string(2);
                vm -> push_string(Vital::Tool::Crypto::hash(mode, input));
                return 1;
            });
        
            API::bind(vm, "crypto", "encode", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto input = vm -> get_string(1);
                vm -> push_string(Vital::Tool::Crypto::encode(input));
                return 1;
            });
        
            API::bind(vm, "crypto", "decode", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto input = vm -> get_string(1);
                vm -> push_string(Vital::Tool::Crypto::decode(input));
                return 1;
            });
        
            API::bind(vm, "crypto", "encrypt", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 3) || (!vm -> is_string(1)) || (!vm -> is_string(2)) || (!vm -> is_string(3))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto mode = vm -> get_string(1);
                auto input = vm -> get_string(2);
                auto key = vm -> get_string(3);
                auto result = Vital::Tool::Crypto::encrypt(mode, input, key);
                vm -> push_string(result.first);
                vm -> push_string(result.second);
                return 2;
            });
        
            API::bind(vm, "crypto", "decrypt", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 4) || (!vm -> is_string(1)) || (!vm -> is_string(2)) || (!vm -> is_string(3)) || (!vm -> is_string(4))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto mode = vm -> get_string(1);
                auto input = vm -> get_string(2);
                auto key = vm -> get_string(3);
                auto iv = vm -> get_string(4);
                vm -> push_string(Vital::Tool::Crypto::decrypt(mode, input, key, iv));
                return 1;
            });
        }
    };
}