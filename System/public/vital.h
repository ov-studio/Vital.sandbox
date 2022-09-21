/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: vital.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Vital Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <pch.h>


///////////////////////
// Namespace: Vital //
///////////////////////

namespace Vital {
    static unsigned int vTick = NULL;
    extern unsigned int getSystemTick();
    extern unsigned int getApplicationTick();
}