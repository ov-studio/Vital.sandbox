/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: rest.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rest APIs
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
    class Rest : public Vital::Tool::Module {
        public:
            inline static void bind(void* instance) {
                auto vm = Vital::Sandbox::Lua::create::toVM(instance);

                API::bind(vm, "rest", "get", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
                    return vm -> execute([&]() -> int {
                        if (!vm -> is_virtual()) throw Vital::Error::fetch("invalid-thread");
                        if ((vm -> getArgCount() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto url = vm -> getString(1);
                        Vital::Tool::Thread([=](Vital::Tool::Thread* thread) -> void {
                            try {
                                vm -> setString(Vital::System::Rest::get(url));
                                vm -> setBool(false);
                            }
                            catch(const std::runtime_error& error) {
                                vm -> setBool(false);
                                vm -> setString(error.what());
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