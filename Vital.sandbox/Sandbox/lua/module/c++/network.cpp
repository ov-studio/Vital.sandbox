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
                bool isPlatformClient = Vital::System::getPlatform() == "client";
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1)) || (!isPlatformClient && (!vm -> isNumber(2)))) throw ErrorCode["invalid-arguments"];
                int stackArgCount = vm -> getArgCount();
                int queryArgCount = stackArgCount - (isPlatformClient ? 2 : 3);
                std::string networkName = vm -> getString(1);
                int peerID = isPlatformClient ? 0 : vm -> getInt(2);
                bool isLatent = vm -> isBool(stackArgCount - queryArgCount - 1) ? vm -> getBool(stackArgCount - queryArgCount - 1) : false;
                Vital::Type::Stack::Instance networkArgs;
                std::cout << "Network name: " << networkName << std::endl;
                networkArgs.push("Network:name", networkName);
                for (int i = stackArgCount - queryArgCount + 1; i <= stackArgCount; i++) {
                    if (vm -> isString(i)) networkArgs.push(vm -> getString(i));
                    else if (vm -> isNumber(i)) networkArgs.push(vm -> getInt(i));
                    else throw ErrorCode["invalid-arguments"];
                }
                Vital::System::Network::emit(networkArgs, peerID, isLatent);
                vm -> setBool(true);
                return 1;
            });
        });
    }
}
