/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: private: engine.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Engine APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/lua/api/public/engine.h>
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Tool/inspect.h>
#include <godot_cpp/variant/utility_functions.hpp>
#endif


///////////////
// Lua: API //
///////////////

void Vital::Sandbox::Lua::API::Engine::bind(void* instance) {
    auto vm = static_cast<vsdk_vm*>(instance);

    API::bind(vm, "engine", "get_platform", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setString(Vital::System::get_platform());
            return 1;
        });
    });

    API::bind(vm, "engine", "get_tick", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(static_cast<int>(Vital::System::get_tick()));
            return 1;
        });
    });

    #if defined(Vital_SDK_Client)
    API::bind(vm, "engine", "get_serial", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setString(Vital::Tool::Inspect::fingerprint());
            return 1;
        });
    });
    #endif

    #if defined(Vital_SDK_Client)
    API::bind(vm, "engine", "print", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            std::ostringstream buffer;
            for (int i = 0; i < vm -> getArgCount(); ++i) {
                size_t length;
                const char* value = luaL_tolstring(ref, i + 1, &length);
                if (i != 0) buffer << " ";
                buffer << std::string(value, length);
                vm -> pop(1);
            }
            godot::UtilityFunctions::print(buffer.str().c_str());
            vm -> setBool(true);
            return 1;
        });
    });
    #endif

    API::bind(vm, "engine", "loadString", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw Vital::Error::fetch("invalid-arguments");
            std::string buffer = vm -> getString(1);
            bool result = false;
            if (vm -> isBool(2)) {
                bool autoload = vm -> getBool(2);
                result = vm -> loadString(buffer, autoload);
                if (result) return 1;
            }
            else result = vm -> loadString(buffer);
            vm -> setBool(result);
            return 1;
        });
    });
}

void Vital::Sandbox::Lua::API::Engine::inject(void* instance) {
    auto vm = static_cast<vsdk_vm*>(instance);
    #if defined(Vital_SDK_Client)
    vm -> getGlobal("engine");
    vm -> getTableField("print", -1);
    vm -> setGlobal("print");
    vm -> pop(1);
    #endif
}
