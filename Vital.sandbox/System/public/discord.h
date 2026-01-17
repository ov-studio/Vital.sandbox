/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: discord.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene, Ianec
     DOC: 15/01/2026
     Desc: Discord Rich Presence System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if defined(Vital_SDK_Client)
#pragma once
#include <string>


////////////////////////////
// Vital: System: Discord //
////////////////////////////

namespace Vital::System::Discord {
    // // Managers //
    extern bool start();
    extern bool stop();

    // // APIs //
    extern bool isConnected();
    extern bool setActivity(
        const std::string& state,
        const std::string& details
    );
}
#endif