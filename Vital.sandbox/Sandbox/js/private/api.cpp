/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: js: private: api.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: API Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/js/public/api.h>


//////////////
// JS: API //
//////////////

namespace Vital::Sandbox::JS {
    void create::hook(const std::string& mode) {
        auto instance = static_cast<void*>(this);
        std::vector<std::pair<std::function<void(void*)>, std::function<void(void*)>>> apis = {};    
        for (auto& i : apis) {
            if (mode == "bind") i.first(instance);
            else if (mode == "inject") i.second(instance);
        }
    }

    namespace API {
        std::function<void(const std::string&)> vsdk_errorhandle = NULL;
        void createErrorHandle(std::function<void(const std::string&)> exec) {
            vsdk_errorhandle = exec;
        }
    
        void error(const std::string& error) {
            if (!vsdk_errorhandle) return;
            vsdk_errorhandle(error);
        }
    
        void bind(vsdk_vm* vm, const std::string& parent, const std::string& name, vsdk_exec exec) {
            vm -> registerFunction(name, exec, parent);
        }
    }
}