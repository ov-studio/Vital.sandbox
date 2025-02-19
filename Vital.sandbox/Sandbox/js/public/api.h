/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: js: public: api.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: API Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/js/public/vm.h>


//////////////
// JS: API //
//////////////

namespace Vital::Sandbox::JS::API {
    // Handlers //
    extern void createErrorHandle(std::function<void(const std::string&)> exec);
    extern void error(const std::string& error);
    extern void bind(vsdk_vm* vm, const std::string& parent, const std::string& name, vsdk_exec exec);
    extern void boot(vsdk_vm* vm);
    extern void inject(vsdk_vm* vm);
}