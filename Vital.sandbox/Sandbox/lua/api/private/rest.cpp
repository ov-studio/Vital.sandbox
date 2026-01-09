/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: private: rest.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: REST APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/lua/api/public/rest.h>
#include <Vital.sandbox/System/public/rest.h>


///////////////
// Lua: API //
///////////////

void Vital::Sandbox::Lua::API::REST::bind(void* instance) {
    auto vm = static_cast<vsdk_vm*>(instance);

    API::bind(vm, "rest", "get", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            if (!vm -> isVirtualThread()) throw std::runtime_error(get_error("invalid-thread"));
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(get_error("invalid-arguments"));
            std::string url = vm -> getString(1);
            Vital::Type::Thread([=](Vital::Type::Thread* thread) -> void {
                try {
                    vm -> setString(Vital::System::REST::get(std::string(url)));
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