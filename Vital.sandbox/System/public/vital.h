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
#include <Type/public/package.h>
#include <Type/public/module.h>
#include <Type/public/thread.h>
#include <Type/public/timer.h>
#include <Type/public/inspect.h>
#include <Type/public/event.h>
#include <Type/public/network.h>
#include <Type/public/math.h>


////////////////////
// Vital: System //
////////////////////

namespace Vital::System {
    extern std::string getSystemPlatform();
    extern std::string getSystemSerial();
    extern unsigned int getSystemTick();
    extern unsigned int getAppTick();
    extern unsigned int getClientTick();
    extern bool resetClientTick();
}