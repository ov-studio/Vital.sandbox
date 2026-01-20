/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: crypto.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Crypto APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/index.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    class Crypto : public Vital::Tool::Module {
        public:
            inline static void bind(void* instance) {
                auto vm = Vital::Sandbox::Lua::Machine::to_vm(instance);

                API::bind(vm, "crypto", "hash", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::Machine::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Error::fetch("invalid-arguments");
                        auto mode = vm -> get_string(1);
                        auto buffer = vm -> get_string(2);
                        vm -> push_string(Vital::Tool::Crypto::hash(mode, buffer));
                        return 1;
                    });
                });
            
                API::bind(vm, "crypto", "encode", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::Machine::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto buffer = vm -> get_string(1);
                        vm -> push_string(Vital::Tool::Crypto::encode(buffer));
                        return 1;
                    });
                });
            
                API::bind(vm, "crypto", "decode", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::Machine::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto buffer = vm -> get_string(1);
                        vm -> push_string(Vital::Tool::Crypto::decode(buffer));
                        return 1;
                    });
                });
            
                API::bind(vm, "crypto", "encrypt", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::Machine::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 3) || (!vm -> is_string(1)) || (!vm -> is_string(2)) || (!vm -> is_string(3))) throw Vital::Error::fetch("invalid-arguments");
                        auto mode = vm -> get_string(1);
                        auto buffer = vm -> get_string(2);
                        auto key = vm -> get_string(3);
                        auto result = Vital::Tool::Crypto::encrypt(mode, buffer, key);
                        vm -> push_string(result.first);
                        vm -> push_string(result.second);
                        return 2;
                    });
                });
            
                API::bind(vm, "crypto", "decrypt", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::Machine::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 4) || (!vm -> is_string(1)) || (!vm -> is_string(2)) || (!vm -> is_string(3)) || (!vm -> is_string(4))) throw Vital::Error::fetch("invalid-arguments");
                        auto mode = vm -> get_string(1);
                        auto buffer = vm -> get_string(2);
                        auto key = vm -> get_string(3);
                        auto iv = vm -> get_string(4);
                        vm -> push_string(Vital::Tool::Crypto::decrypt(mode, buffer, key, iv));
                        return 1;
                    });
                });
            }
    };
}