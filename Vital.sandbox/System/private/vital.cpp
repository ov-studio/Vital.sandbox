/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: vital.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vital System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>


////////////////////
// Vital: System //
////////////////////

namespace Vital::System {
    const std::string& getPlatform() {
        return "server";
    }

    unsigned int getSystemTick() {
        return static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()/1000000);
    }

    unsigned int getSystemTick() {
        return static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()/1000000);
    }

    unsigned int vApplicationTick;
    unsigned int getApplicationTick() {
        vApplicationTick = vApplicationTick ? vApplicationTick : getSystemTick();
        return static_cast<unsigned int>(getSystemTick() - vApplicationTick);
    }

    unsigned int vClientTick;
    unsigned int getClientTick() {
        vClientTick = vClientTick ? vClientTick : getApplicationTick();
        return static_cast<unsigned int>(getSystemTick() - vClientTick);
    }

    bool resetClientTick() {
        vClientTick = getApplicationTick();
        return true;
    }
}