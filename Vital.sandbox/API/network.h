/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: network.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>


//////////////////////////
// Vital: API: Network //
//////////////////////////

namespace Vital::Sandbox::API {
    struct Network : vm_module {
        inline static const std::string base_name = "network";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "emit", [](auto vm) -> int {
                bool client = get_platform() == "client";
                if ((vm -> get_count() < 1) || (!vm -> is_string(1)) || (!client && (!vm -> is_number(2)))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
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
                //Vital::System::Network::emit(arguments, peerID, isLatent);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> get_global(base_name);
            vm -> get_table_field("execute", -1);
            vm -> set_reference("vital.network:execute", -1);
            vm -> pop(2);
            vm -> table_set_nil("execute", base_name);
        }
    };
}