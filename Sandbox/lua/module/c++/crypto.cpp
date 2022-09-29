/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: module: c++: crypto.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Crypto Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/crypto.h>
#include <Sandbox/lua/public/api.h>


/////////////////////////
// Lua: Crypto Binder //
/////////////////////////

namespace Vital::Lua::API {
    void vSandbox_Crypto() {
        bind("crypto", "sha1", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) vm -> setBool(false);
            else {
                std::string buffer = vm -> getString(1);
                std::string hash = Vital::Crypto::SHA1(buffer);
                vm -> setString(hash);
            }
            return 1;
        });

        bind("crypto", "sha224", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) vm -> setBool(false);
            else {
                std::string buffer = vm -> getString(1);
                std::string hash = Vital::Crypto::SHA224(buffer);
                vm -> setString(hash);
            }
            return 1;
        });

        bind("crypto", "sha256", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) vm -> setBool(false);
            else {
                std::string buffer = vm -> getString(1);
                std::string hash = Vital::Crypto::SHA256(buffer);
                vm -> setString(hash);
            }
            return 1;
        });

        bind("crypto", "sha384", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm->getArgCount() < 1) || (!vm -> isString(1))) vm -> setBool(false);
            else {
                std::string buffer = vm -> getString(1);
                std::string hash = Vital::Crypto::SHA384(buffer);
                vm -> setString(hash);
            }
            return 1;
        });

        bind("crypto", "sha512", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) vm -> setBool(false);
            else {
                std::string buffer = vm -> getString(1);
                std::string hash = Vital::Crypto::SHA512(buffer);
                vm -> setString(hash);
            }
            return 1;
        });

        bind("crypto", "sha512", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) vm -> setBool(false);
            else {
                std::string buffer = vm -> getString(1);
                std::string hash = Vital::Crypto::SHA512(buffer);
                vm -> setString(hash);
            }
            return 1;
        });


        bind("crypto", "encrypt", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 3) || (!vm -> isString(1)) || (!vm->isString(2)) || (!vm->isString(3))) vm -> setBool(false);
            else {
                std::string mode = vm->getString(1);
                std::string buffer = vm -> getString(2);
                std::string key = vm->getString(3);
                try {
                    auto result = Vital::Crypto::encrypt(mode, buffer, key);
                }
                catch (int error) { vm->setBool(false); }
            }
            return 1;
        });
    }
}
