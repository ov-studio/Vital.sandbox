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
    extern std::map<vsdk_bind, vsdk_exec> vsdk_binds;
    extern bool createErrorHandle(std::function<void(const std::string&)> exec);
    extern bool error(const std::string& error);
    extern bool bind(const std::string& parent, const std::string& name, vsdk_exec exec);
    extern bool unbind(const std::string& parent, const std::string& name);

    // Booter //
    extern bool boot();
}