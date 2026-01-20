/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: rest.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rest APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>


////////////////////////////////
// Vital: Sandbox: API: Rest //
////////////////////////////////

namespace Vital::Sandbox::API {
    class Rest : public Vital::Tool::Module {
        public:
            inline static void bind(void* machine) {
                auto vm = Machine::to_machine(machine);

                API::bind(vm, "rest", "get", [](auto* ref) -> int {
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        if (!vm -> is_virtual()) throw Vital::Error::fetch("invalid-thread");
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto url = vm -> get_string(1);
                        Vital::Tool::Thread([=](Vital::Tool::Thread* thread) -> void {
                            try {
                                vm -> push_string(Vital::Tool::Rest::get(url));
                                vm -> push_bool(false);
                            }
                            catch(const std::runtime_error& error) {
                                vm -> push_bool(false);
                                vm -> push_string(error.what());
                            }
                            vm -> resume(2);
                        }).detach();
                        vm -> pause();
                        return 0;
                    });
                });
            }
    };
}