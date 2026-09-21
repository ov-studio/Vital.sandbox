/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: registry.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Registry
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Vital/tool.h>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    struct vm_instance_base;
    inline std::unordered_map<void*, vm_instance_base*> vm_node_registry;
    inline std::mutex vm_node_registry_mutex;
}
