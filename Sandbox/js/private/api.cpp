/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: js: private: api.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
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

namespace Vital::JS::API {
    // Handlers //
    std::map<vital_exec_ref, vital_exec> vMethods;
    std::function<void(std::string&)> onErrorHandler = NULL;
    bool onErrorHandle(std::function<void(std::string&)> exec) {
        onErrorHandler = exec;
        return true;
    }

    // Helpers //
    bool error(std::string& error) {
        if (!onErrorHandler) return false;
        onErrorHandler(error);
        return true;
    }

    bool bind(std::string parent, std::string name, vital_exec exec) {
        vital_exec_ref ref = { parent, name };
        vMethods[ref] = exec;
        return true;
    }
    bool unbind(std::string parent, std::string name) {
        vital_exec_ref ref = { parent, name };
        vMethods.erase(ref);
        return true;
    }

    // Booter //
    bool boot() {
        // TODO: EXPOSE LATER
        //vSandbox_Engine();
        //vSandbox_Crypto();
        //vSandbox_File();
        return true;
    }
}