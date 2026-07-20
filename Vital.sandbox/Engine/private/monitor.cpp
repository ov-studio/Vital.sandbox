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

        vm -> get_raw_reference(it -> second.exec_ref);
        vm -> call(0, 1);

        std::unordered_set<const void*> visited;
        auto value = vm -> collect_value(vm -> get_count(), visited).to_variant();
        vm -> pop(1);
        return value;
    }
}