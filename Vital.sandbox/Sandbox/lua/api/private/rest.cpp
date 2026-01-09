/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: private: rest.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rest APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/lua/api/public/rest.h>
#include <Vital.sandbox/Tool/rest.h>


///////////////
// Lua: API //
///////////////

void Vital::Sandbox::Lua::API::Rest::bind(void* instance) {
    auto vm = static_cast<vsdk_vm*>(instance);

    API::bind(vm, "rest", "get", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            if (!vm -> isVirtualThread()) throw Vital::Error::fetch("invalid-thread");
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw Vital::Error::fetch("invalid-arguments");
            std::string url = vm -> getString(1);
            Vital::Tool::Thread([=](Vital::Tool::Thread* thread) -> void {
                try {
                    vm -> setString(Vital::System::Rest::get(std::string(url)));
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