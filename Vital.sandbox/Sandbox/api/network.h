/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: network.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>
#include <Vital.sandbox/System/public/network.h>


///////////////////////////////////
// Vital: Sandbox: API: Network //
///////////////////////////////////

namespace Vital::Sandbox::API {
    class Network : public Vital::Tool::Module {
        public:
            static void bind(void* machine) {
                auto vm = Machine::to_machine(machine);

                API::bind(vm, "network", "emit", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        bool client = get_platform() == "client";
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1)) || (!client && (!vm -> is_number(2)))) throw Vital::Error::fetch("invalid-arguments");
                        int queryArg = client ? 3 : 4;
                        auto name = vm -> get_string(1);
                        int peerID = client ? 0 : vm -> get_int(2);
                        bool isLatent = vm -> is_bool(queryArg - 1) ? vm -> get_bool(queryArg - 1) : false;
                        auto payload = vm -> is_string(queryArg) ? vm -> get_string(queryArg) : "";
                        Vital::Tool::Stack arguments;
                        arguments.object["network.name"] = Vital::Tool::StackValue(name);
                        arguments.object["network.payload"] = Vital::Tool::StackValue(payload);
                        /*
                        Usage Example:
                        Vital::Tool::Stack arguments;
                        arguments.array.emplace_back(42);
                        arguments.array.emplace_back(3.14f);
                        arguments.array.emplace_back(2.718);
                        arguments.object["pi"] = Vital::Tool::StackValue(3.14159);
                        arguments.object["answer"] = Vital::Tool::StackValue(42);
                        std::string data = arguments.serialize();
                        Vital::Tool::Stack restored = Vital::Tool::Stack::deserialize(data);
        
                        int number = restored.array[0].as<int32_t>();
                        godot::UtilityFunctions::print(number);
                        */
                        Vital::System::Network::emit(arguments, peerID, isLatent);
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            }

            static void inject(void* machine) {
                auto vm = Machine::to_machine(machine);
                vm -> get_global("network");
                vm -> get_table_field("execute", -1);
                vm -> push_reference("vital.network:execute", -1);
                vm -> pop(2);
                vm -> table_set_nil("execute", "network");
            }

            static void execute(const std::string& name, const std::string& payload) {
                for (auto vm : Machine::fetch_buffer()) {
                    if (!vm.second -> is_virtual()) {
                        vm.second -> get_reference("vital.network:execute", true);
                        vm.second -> push_string(name);
                        vm.second -> push_string(payload);
                        vm.second -> pcall(2, 0);
                    }
                }
            }
    };
}