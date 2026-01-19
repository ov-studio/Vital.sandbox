/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: private: api.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: API Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/index.h>



///////////////////////////////
// Vital: Sandbox: Lua: API //
///////////////////////////////

namespace Vital::Sandbox::Lua {
    namespace API {
        std::function<void(const std::string&)> vsdk_errorhandle = NULL;
        void createErrorHandle(std::function<void(const std::string&)> exec) {
            vsdk_errorhandle = exec;
        }
    
        void error(const std::string& error) {
            if (!vsdk_errorhandle) return;
            vsdk_errorhandle(error);
        }
    
        void bind(create* vm, const std::string& parent, const std::string& name, vm_exec exec) {
            vm -> table_set_function(name, exec, parent);
        }
    }
}