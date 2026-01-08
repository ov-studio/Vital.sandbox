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
#include <Type/public/inspect.h>
#include <Type/public/event.h>
#include <Type/public/network.h>


////////////////////
// Vital: System //
////////////////////

namespace Vital::System {
    extern std::string getPlatform();
    extern unsigned int getTick();

    #if defined(Vital_SDK_Client)
        #if defined(Vital_SDK_WINDOWS)
            extern std::string getSerial();
        #endif
    #endif
}