/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: js: public: api.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
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
    extern std::map<vital_exec_ref, vital_exec> vMethods;
    extern bool onErrorHandle(std::function<void(std::string&)> exec);

    // Helpers //
    extern bool error(std::string& error);
    extern bool bind(std::string parent, std::string name, vital_exec exec);
    extern bool unbind(std::string parent, std::string name);

    // Booter //
    extern bool boot();

    // Binds //
    // TODO: EXPOSE LATER
    //extern void vSandbox_Engine();
    //extern void vSandbox_Crypto();
    //extern void vSandbox_File();
}