/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: network.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/lua/public/vm.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    class Network : public Vital::Tool::Module {
        public:
            static void bind(void* instance) {
                auto vm = Vital::Sandbox::Lua::create::toVM(instance);

                API::bind(vm, "network", "emit", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
                    return vm -> execute([&]() -> int {
                        bool client = get_platform() == "client";
                        if ((vm -> getArgCount() < 1) || (!vm -> is_string(1)) || (!client && (!vm -> is_number(2)))) throw Vital::Error::fetch("invalid-arguments");
                        int queryArg = client ? 3 : 4;
                        auto name = vm -> getString(1);
                        int peerID = client ? 0 : vm -> getInt(2);
                        bool isLatent = vm -> is_bool(queryArg - 1) ? vm -> getBool(queryArg - 1) : false;
                        auto payload = vm -> is_string(queryArg) ? vm -> getString(queryArg) : "";
                        Vital::Tool::Stack networkArgs;
                        networkArgs.push("Network:name", name);
                        networkArgs.push("Network:payload", payload);
                        Vital::System::Network::emit(networkArgs, peerID, isLatent);
                        vm -> setBool(true);
                        return 1;
                    });
                });
            }

            static void inject(void* instance) {
                auto vm = Vital::Sandbox::Lua::create::toVM(instance);

                vm -> getGlobal("network");
                vm -> getTableField("execNetwork", -1);
                vm -> setReference(Vital::Tool::Crypto::hash("SHA256", "network.execNetwork"), -1);
                vm -> pop(2);
                vm -> registerNil("execNetwork", "network");
            }

            static void execute(const std::string& name, const std::string& payload) {
                for (auto vm : Vital::Sandbox::Lua::create::fetchVMs()) {
                    if (!vm.second -> is_virtual()) {
                        vm.second -> getReference(Vital::Tool::Crypto::hash("SHA256", "network.execNetwork"), true);
                        vm.second -> setString(name);
                        vm.second -> setString(payload);
                        vm.second -> pcall(2, 0);
                    }
                }
            }
    };
}