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
    void Network::bind(void* instance) {
        auto vm = static_cast<vsdk_vm*>(instance);

        API::bind(vm, "network", "emit", [](auto* ref) -> int {
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

    void Network::inject(void* instance) {
        auto vm = static_cast<vsdk_vm*>(instance);
        vm -> getGlobal("network");
        vm -> getTableField("execNetwork", -1);
        vm -> setReference("execNetwork", -1);
    }
    
    void Network::execute(void* instance) {
        auto vm = static_cast<vsdk_vm*>(instance);
        auto reference = vm -> getReference("execNetwork");
        std::cout << "Network exec ref: " << reference << std::endl;
        //lua_rawgeti(vm, LUA_REGISTRYINDEX, reference); // TODO: ADD WAY TO GET REGISTRY FUNCTION...
        //setNil();
        //lua_pcall // 1 arg, 1 return
        //lua_pcall(vm, 1, 1, 0);
        //pop(2);
        //std::cout << "IS FUNCTION: " << reference << std::endl;
        //std::cout << "ARGS: " << getArgCount() << std::endl;
    }
}