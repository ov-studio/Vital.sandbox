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
#include <Sandbox/lua/api/public/engine.h>
#if defined(Vital_SDK_Client)
#include <godot_cpp/variant/utility_functions.hpp>
#endif


///////////////
// Lua: API //
///////////////

void Vital::Sandbox::Lua::API::Engine::bind(void* instance) {
    auto vm = static_cast<vsdk_vm*>(instance);

    API::bind(vm, "engine", "getPlatform", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setString(Vital::System::getPlatform());
            return 1;
        });
    });

    API::bind(vm, "engine", "getTick", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setNumber(static_cast<int>(Vital::System::getTick()));
            return 1;
        });
    });

    #if defined(Vital_SDK_Client)
    API::bind(vm, "engine", "getSerial", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            vm -> setString(Vital::System::getSerial());
            return 1;
        });
    });
    #endif

    #if defined(Vital_SDK_Client)
    API::bind(vm, "engine", "print", [](auto* ref) -> int {
        auto vm = fetchVM(ref);
        return vm -> execute([&]() -> int {
            godot::UtilityFunctions::print("YESS!!! WORKS");
            std::vector<std::string> args;
            std::ostringstream buffer;
            for (int i = 0; i < vm -> getArgCount(); ++i) {
                args.push_back(vm -> getString(i + 1));
            }
            for (size_t i = 0; i < args.size(); ++i) {
                if (i != 0) buffer << " ";
                buffer << args[i];
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
            if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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