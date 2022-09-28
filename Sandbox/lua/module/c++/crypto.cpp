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
        bind("crypto", "sha256", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(-1))) {
                vm -> setBool(false);
            }
            else {
                std::string buffer = vm->getString(-1);
                vm -> setString(Vital::Crypto::SHA256(buffer));
            }
            return 1;
        });
    }
}
