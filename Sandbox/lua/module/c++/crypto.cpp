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

namespace Vital::Sandbox::Lua::API {
    void vSandbox_Crypto() {
        bind("crypto", "hash", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 2) || (!vm -> isString(1)) || (!vm -> isString(2))) vm -> setBool(false);
            else {
                try {
                    std::string mode = vm -> getString(1);
                    std::string buffer = vm -> getString(2);
                    auto result = Vital::System::Crypto::hash(mode, buffer);
                    vm -> setString(result);
                }
                catch(std::string error) {
                    vm -> setBool(false);
                    vm -> throwError(error);
                }
            }
            return 1;
        });

        bind("crypto", "encode", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) vm -> setBool(false);
            else {
                try {
                    std::string buffer = vm -> getString(1);
                    auto result = Vital::System::Crypto::encode(buffer);
                    vm -> setString(result);
                }
                catch(std::string error) {
                    vm -> setBool(false);
                    vm -> throwError(error);
                }
            }
            return 1;
        });

        bind("crypto", "decode", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) vm -> setBool(false);
            else {
                try {
                    std::string buffer = vm -> getString(1);
                    auto result = Vital::System::Crypto::decode(buffer);
                    vm -> setString(result);
                }
                catch(std::string error) {
                    vm -> setBool(false);
                    vm -> throwError(error);
                }
            }
            return 1;
        });

        bind("crypto", "encrypt", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 3) || (!vm -> isString(1)) || (!vm -> isString(2)) || (!vm -> isString(3))) vm -> setBool(false);
            else {
                try {
                    std::string mode = vm -> getString(1);
                    std::string buffer = vm -> getString(2);
                    std::string key = vm -> getString(3);
                    auto result = Vital::System::Crypto::encrypt(mode, buffer, key);
                    vm -> setString(result.first);
                    vm -> setString(result.second);
                    return 2;
                }
                catch(std::string error) {
                    vm -> setBool(false);
                    vm -> throwError(error);
                }
            }
            return 1;
        });

        bind("crypto", "decrypt", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 4) || (!vm -> isString(1)) || (!vm -> isString(2)) || (!vm -> isString(3)) || (!vm -> isString(4))) vm -> setBool(false);
            else {
                try {
                    std::string mode = vm -> getString(1);
                    std::string buffer = vm -> getString(2);
                    std::string key = vm -> getString(3);
                    std::string iv = vm -> getString(4);
                    auto result = Vital::System::Crypto::decrypt(mode, buffer, key, iv);
                    vm -> setString(result);
                }
                catch(std::string error) {
                    vm -> setBool(false);
                    vm -> throwError(error);
                }
            }
            return 1;
        });
    }
}
