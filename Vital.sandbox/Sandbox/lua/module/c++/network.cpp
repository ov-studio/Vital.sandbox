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

        bind("network", "emit", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                bool isClient = Vital::System::getSystemPlatform() == "client";
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1)) || (!isClient && (!vm -> isNumber(2)))) throw ErrorCode["invalid-arguments"];
                int queryArg = isClient ? 3 : 4;
                std::string name = vm -> getString(1);
                int peerID = isClient ? 0 : vm -> getInt(2);
                bool isLatent = vm -> isBool(queryArg - 1) ? vm -> getBool(queryArg - 1) : false;
                std::string payload = vm -> isString(queryArg) ? vm -> getString(queryArg) : "";
                Vital::Type::Stack networkArgs;
                networkArgs.push("Network:name", name);
                networkArgs.push("Network:payload", payload);
                Vital::System::Network::emit(networkArgs, peerID, isLatent);
                vm -> setBool(true);
                return 1;
            });
        });
    }
}
