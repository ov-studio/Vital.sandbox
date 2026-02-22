/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: index.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/log.h>
#include <Vital.sandbox/Tool/stack.h>
#include <Vital.sandbox/Tool/module.h>
#include <Vital.sandbox/Tool/thread.h>
#include <Vital.sandbox/Tool/timer.h>
#include <Vital.sandbox/Tool/file.h>
#include <Vital.sandbox/Tool/event.h>
#include <Vital.sandbox/Tool/network.h>
#include <Vital.sandbox/Tool/rest.h>
#include <Vital.sandbox/Tool/inspect.h>
#include <Vital.sandbox/Tool/crypto.h>
#include <Vital.sandbox/Tool/shrinker.h>
#include <Vital.sandbox/Vendor/lua/lua.hpp>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    class Machine;
    using vm_state = lua_State;
    using vm_exec = lua_CFunction;
    using vm_machines = std::unordered_map<vm_state*, Machine*>;
    using vm_refs = std::unordered_map<std::string, int>;
    using vm_bind = std::function<int(Machine*)>;

    struct vm_api {
        std::function<void(Machine*)> bind;
        std::function<void(Machine*)> inject;
    };
    using vm_apis = std::vector<vm_api>;

    struct vm_module {
        static void bind(Machine* vm) {}
        static void inject(Machine* vm) {}

        template<typename T>
        static vm_api make_api() {
            return {
                [](Machine* vm) { T::bind(vm); },
                [](Machine* vm) { T::inject(vm); }
            };
        }
    };

    namespace API {
        extern void log(const std::string& type, const std::string& message);
        extern void bind(Machine* vm, const std::string& nspace, const std::string& name, vm_bind exec);
    }
}