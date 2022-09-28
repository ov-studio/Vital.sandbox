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
    static const std::string vSignature = "VitalSDK";
    static unsigned int vTick;
    extern unsigned int getSystemTick();
    extern unsigned int getApplicationTick();
    extern std::string sha256(std::string str);
    //extern std::string md5(std::string& str);
}