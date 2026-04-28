/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: shrinker.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Shrinker APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>


///////////////////////////
// Vital: API: Shrinker //
///////////////////////////

namespace Vital::Sandbox::API {
    struct Shrinker : vm_module {
        inline static const std::string base_name = "shrinker";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "compress", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(input)")
                    .require(1, &Machine::is_string);

                auto input = vm -> get_string(1);
                vm -> push_value(Tool::Shrinker::compress(input));
                return 1;
            });

            API::bind(vm, {base_name}, "decompress", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(input)")
                    .require(1, &Machine::is_string);

                auto input = vm -> get_string(1);
                vm -> push_value(Tool::Shrinker::decompress(input));
                return 1;
            });
        }
    };
}