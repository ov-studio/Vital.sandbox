/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: vital.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Vital Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>


///////////////////////
// Namespace: Vital //
///////////////////////

namespace Vital {
    unsigned int getSystemTick() {
        return static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count() / 1000000);
    }
    unsigned int getApplicationTick() {
        vTick = vTick ? vTick : getSystemTick();
        return static_cast<unsigned int>(getSystemTick() - vTick);
    }
}