/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: index.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Module Initializer
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <chrono>


///////////////////////
// Namespace: Vital //
///////////////////////

namespace Vital {
    static const unsigned int getSystemTick() {
        return std::chrono::high_resolution_clock::now().time_since_epoch().count()/1000000;
    }
    static const const unsigned int vTick = getSystemTick();

    static const unsigned int getApplicationTick() {
        return getSystemTick() - vTick;
    }
}