/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: module: c++: network.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/network.h>
#include <Sandbox/lua/public/api.h>


//////////////////////////
// Lua: Network Binder //
//////////////////////////

namespace Vital::Sandbox::Lua::API {
    static bool isBound = false;
    void vSandbox_Network() {
        if (isBound) return; isBound = true;

        bind("network", "emit", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw ErrorCode["invalid-arguments"];
                bool isPlatformClient = Vital::System::getPlatform() == "client";
                if (!isPlatformClient && (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                int queryArgCount = isPlatformClient ? (vm -> getArgCount()) - 1 : (vm -> getArgCount()) - 2;
                std::string networkName = vm -> getString(1);
                int peerID = isPlatformClient ? 0 : vm -> getInt(2);
                Vital::System::Network::emit(networkName, peerID);
                vm -> setBool(true);
                return 1;
            });
        });
    }
}
