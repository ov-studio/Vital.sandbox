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

namespace Vital::Sandbox::Lua::API {
    bool Crypto::bound = false;

    void Crypto::boot() {
        if (bound) return;
        bound = true;

        bind("crypto", "hash", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isString(1)) || (!vm -> isString(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                std::string mode = vm -> getString(1);
                std::string buffer = vm -> getString(2);
                vm -> setString(Vital::System::Crypto::hash(mode, buffer));
                return 1;
            });
        });

        bind("crypto", "encode", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                std::string buffer = vm -> getString(1);
                vm -> setString(Vital::System::Crypto::encode(buffer));
                return 1;
            });
        });

        bind("crypto", "decode", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                std::string buffer = vm -> getString(1);
                vm -> setString(Vital::System::Crypto::decode(buffer));
                return 1;
            });
        });

        bind("crypto", "encrypt", [](vsdk_ref* ref) -> int {
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

        bind("crypto", "decrypt", [](vsdk_ref* ref) -> int {
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
}