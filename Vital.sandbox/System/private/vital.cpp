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
    unsigned int applicationTick, clientTick;

    std::string getPlatform() {
        return "server";
    }

    unsigned int getSystemTick() {
        return static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()/1000000);
    }

    unsigned int getApplicationTick() {
        applicationTick = applicationTick ? applicationTick : getSystemTick();
        return getSystemTick() - applicationTick;
    }

    unsigned int getClientTick() {
        clientTick = clientTick ? clientTick : getApplicationTick();
        return getSystemTick() - clientTick;
    }

    bool resetClientTick() {
        clientTick = getApplicationTick();
        return true;
    }
}