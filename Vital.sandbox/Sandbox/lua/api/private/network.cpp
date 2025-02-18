/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: private: network.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/lua/api/public/network.h>
#include <System/public/network.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    bool Network::bound = false;

    void Network::boot() {
        if (bound) return;
        bound = true;

        bind("network", "emit", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                bool isClient = Vital::System::getSystemPlatform() == "client";
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1)) || (!isClient && (!vm -> isNumber(2)))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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