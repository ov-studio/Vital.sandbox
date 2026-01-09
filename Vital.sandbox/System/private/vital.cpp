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
#include <System/public/inspect.h>
#include <System/public/crypto.h>


////////////////////
// Vital: System //
////////////////////

namespace Vital::System {
    std::string getPlatform() { 
        #if defined(Vital_SDK_Client)
            return "client";
        #else
            return "server";
        #endif
    }

    unsigned int getTick() {
        return static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()/1000000);
    }

    #if defined(Vital_SDK_Client)
        std::string vsdk_serial = "";
        std::string getSerial() {
            vsdk_serial = vsdk_serial.empty() ? Vital::System::Inspect::fingerprint() : vsdk_serial;
            if (vsdk_serial.empty()) throw std::runtime_error(ErrorCode["serial-nonexistent"]);
            return vsdk_serial;
        }
    #endif
}