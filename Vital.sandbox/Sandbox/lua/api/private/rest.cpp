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
#include <Sandbox/lua/api/public/rest.h>
#include <System/public/rest.h>

#include <godot_cpp/variant/utility_functions.hpp>

///////////////
// Lua: API //
///////////////

void Vital::Sandbox::Lua::API::REST::bind(void* instance) {
    auto vm = static_cast<vsdk_vm*>(instance);

    API::bind(vm, "rest", "get", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            godot::UtilityFunctions::print("executing 1!");
            if (!vm -> isVirtualThread()) throw std::runtime_error(ErrorCode["invalid-thread"]);
            godot::UtilityFunctions::print("executing 2!");
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            std::string url = vm -> getString(1);
            godot::UtilityFunctions::print("executing 3!");

            Vital::Type::Thread([=](Vital::Type::Thread* thread) -> void {
                try {
                    std::string response = Vital::System::REST::get(std::string(url));
                    godot::UtilityFunctions::print(response.c_str());
                }
                catch(const std::runtime_error& error) { godot::UtilityFunctions::print(error.what()); }
                vm -> resume();
            }).detach();
            vm -> pause();
            return 2;
        });
    });
}