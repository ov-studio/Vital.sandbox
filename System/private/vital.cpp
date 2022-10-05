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


////////////
// Vital //
////////////

std::map<std::string, std::string> Vital::ErrorCodes = {
    {"request_failed", "Failed to process specified request"},
    {"decode-failed", "Failed to decode specified data"},
    {"hash-mode-nonexistent", "Invalid hash mode"},
    {"cipher-mode-nonexistent", "Invalid cipher mode"},
    {"cipher-invalid-key", "Invalid cipher key"},
    {"cipher-invalid-iv", "Invalid cipher IV"},
    {"invalid-3d-sound", "Invalid 3D sound"},
    {"file-nonexistent", "File non-existent"}
}

namespace Vital::System {
    unsigned int getSystemTick() {
        return static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count() / 1000000);
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