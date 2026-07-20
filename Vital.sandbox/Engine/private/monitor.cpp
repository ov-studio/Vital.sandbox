/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: monitor.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Monitor Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/monitor.h>
#include <Vital.sandbox/API/utility/monitor.h>
#include <Vital.sandbox/Manager/public/sandbox.h>


/////////////////////////////
// Vital: Engine: Monitor //
/////////////////////////////

namespace Vital::Engine {
    godot::Variant Monitor::dispatch(const godot::String& key) {
        auto std_key = std::string(key.utf8().get_data());
        auto it = Sandbox::API::Monitor::buffer.find(std_key);
        if (it == Sandbox::API::Monitor::buffer.end()) return 0;
        auto vm = Manager::Sandbox::get_singleton() -> get_vm();
        if (!vm) return 0;

        godot::Variant result = 0;
        vm -> execute([&]() -> int {
            vm -> get_raw_reference(it -> second.exec_ref);
            if (!vm -> call(0, 1)) return 0;
            if (!vm -> is_number(vm -> get_count())) {
                vm -> log("error", "monitor callback must return a number");
                return 0;
            }
            result = vm -> get_double(vm -> get_count());
            vm -> pop(1);
            return 0;
        });
        return result;
    }
}