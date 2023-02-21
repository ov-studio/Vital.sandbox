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
#include <Type/public/inspect.h>
#include <Type/public/network.h>
#include <Type/public/math.h>
#include <Type/public/audio.h>


////////////////////
// Vital: System //
////////////////////

namespace Vital::System {
    extern std::string getPlatform();
    extern unsigned int getSystemTick();
    extern unsigned int getApplicationTick();
    extern unsigned int getClientTick();
    extern bool resetClientTick();
}