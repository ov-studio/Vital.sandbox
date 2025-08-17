/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: private: environment.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Environment APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/api/public/environment.h>


///////////////////////////////////////////////////
// Vital: Godot: Sandbox: Lua: API: Environment //
///////////////////////////////////////////////////

void Vital::Godot::Sandbox::Lua::API::Environment::bind(void* instance) {
    auto vm = static_cast<Vital::Sandbox::Lua::vsdk_vm*>(instance);

    Vital::Sandbox::Lua::API::bind(vm, "environment", "get_ssao_intensity", [](auto* ref) -> int {
        auto vm = Vital::Sandbox::Lua::fetchVM(ref);
        return vm -> execute([&]() -> int {
            auto environment = Vital::Godot::Engine::Singleton::get_environment();
            float ssao_intensity = environment -> get_ssao_intensity();
            godot::UtilityFunctions::print("SSAO Intensity: ", ssao_intensity);
            //if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
            //std::string path = vm -> getString(1);
            //vm -> setString(path);
            return 1;
        });
    });
}