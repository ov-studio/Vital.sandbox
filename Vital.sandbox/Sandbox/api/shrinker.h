/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: shrinker.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Shrinker APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>


////////////////////////////////////
// Vital: Sandbox: API: Shrinker //
////////////////////////////////////

namespace Vital::Sandbox::API {
    class Shrinker : public Vital::Tool::Module {
        public:
            inline static void bind(void* machine) {
                auto vm = Machine::to_machine(machine);

                API::bind(vm, "shrinker", "compress", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                        auto input = vm -> get_string(1);
                        vm -> push_string(Vital::Tool::Shrinker::compress(input));
                        return 1;
                    });
                });
            
                API::bind(vm, "shrinker", "decompress", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                        auto input = vm -> get_string(1);
                        vm -> push_string(Vital::Tool::Shrinker::decompress(input));
                        return 1;
                    });
                });
            }
    };
}