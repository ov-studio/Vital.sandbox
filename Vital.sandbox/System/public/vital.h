/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: vital.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vital System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>
#include <Type/public/stack.h>
#include <Type/public/module.h>
#include <Type/public/thread.h>
#include <Type/public/timer.h>
#include <Type/public/event.h>
#include <Type/public/network.h>


////////////////////
// Vital: System //
////////////////////

namespace Vital::System {
    static inline const std::string get_platform() { 
        #if defined(Vital_SDK_Client)
            return "client";
        #else
            return "server";
        #endif
    }

    static inline unsigned int get_tick() {
        return static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()/1000000);
    }
}