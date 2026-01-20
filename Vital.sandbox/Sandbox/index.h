/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: index.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/error.h>
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
#include <Vital.sandbox/Vendor/lua/lua.hpp>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    class Machine;
    using vm_state = lua_State;
    using vm_exec = lua_CFunction;
    using vm_buffer = std::unordered_map<vm_state*, Machine*>;
    using vm_refs = std::unordered_map<std::string, int>;
    using vm_apis = std::vector<std::pair<std::function<void(void*)>, std::function<void(void*)>>>;

    namespace API {
        extern void error(const std::string& error);
        extern void bind(Machine* vm, const std::string& nspace, const std::string& name, vm_exec exec);
    }
}