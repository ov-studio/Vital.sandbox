/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: private: crypto.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Crypto APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/lua/api/public/crypto.h>
#include <System/public/crypto.h>


///////////////
// Lua: API //
///////////////

void Vital::Sandbox::Lua::API::Crypto::bind(void* instance) {
    auto vm = static_cast<vsdk_vm*>(instance);

    API::bind(vm, "crypto", "hash", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 2) || (!vm -> isString(1)) || (!vm -> isString(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            std::string mode = vm -> getString(1);
            std::string buffer = vm -> getString(2);
            vm -> setString(Vital::System::Crypto::hash(mode, buffer));
            return 1;
        });
    });

    API::bind(vm, "crypto", "encode", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            std::string buffer = vm -> getString(1);
            vm -> setString(Vital::System::Crypto::encode(buffer));
            return 1;
        });
    });

    API::bind(vm, "crypto", "decode", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            std::string buffer = vm -> getString(1);
            vm -> setString(Vital::System::Crypto::decode(buffer));
            return 1;
        });
    });

    API::bind(vm, "crypto", "encrypt", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 3) || (!vm -> isString(1)) || (!vm -> isString(2)) || (!vm -> isString(3))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            std::string mode = vm -> getString(1);
            std::string buffer = vm -> getString(2);
            std::string key = vm -> getString(3);
            auto result = Vital::System::Crypto::encrypt(mode, buffer, key);
            vm -> setString(result.first);
            vm -> setString(result.second);
            return 2;
        });
    });

    API::bind(vm, "crypto", "decrypt", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 4) || (!vm -> isString(1)) || (!vm -> isString(2)) || (!vm -> isString(3)) || (!vm -> isString(4))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            std::string mode = vm -> getString(1);
            std::string buffer = vm -> getString(2);
            std::string key = vm -> getString(3);
            std::string iv = vm -> getString(4);
            vm -> setString(Vital::System::Crypto::decrypt(mode, buffer, key, iv));
            return 1;
        });
    });
}