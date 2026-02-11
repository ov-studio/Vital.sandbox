/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: adjustment.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Adjustment APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/index.h>
#include <Vital.extension/Engine/public/console.h>


//////////////////////////////////////
// Vital: Sandbox: API: Adjustment //
//////////////////////////////////////

namespace Vital::Sandbox::API {
    class Console : public Vital::Tool::Module {
        public:
            inline static void bind(void* machine) {
                auto vm = Machine::to_machine(machine);
                
                API::bind(vm, "engine", "print", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        std::ostringstream buffer;
                        for (int i = 0; i < vm -> get_arg_count(); ++i) {
                            size_t length;
                            const char* value = luaL_tolstring(ref, i + 1, &length);
                            if (i != 0) buffer << " ";
                            buffer << std::string(value, length);
                            vm -> pop(1);
                        }
                        Vital::print("info", buffer.str());
                        vm -> push_bool(true);
                        return 1;
                    });
                });
            }
    };
}