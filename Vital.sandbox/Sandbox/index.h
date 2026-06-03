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
#include <Vital.sandbox/Sandbox/machine.h>
#include <Vital.sandbox/Sandbox/runtime/types.h>
#include <Vital.sandbox/Sandbox/runtime/module.h>
#include <Vital.sandbox/Sandbox/runtime/instance.h>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
     namespace API {
         extern void log(const std::string& mode, const std::string& message);
         extern void bind(Machine* vm, const std::vector<std::string>& scope, const std::string& name, vm_bind exec);
     }
}
 