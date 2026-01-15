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

#pragma once
#include <string>


////////////////////////////
// Vital: System: Discord //
////////////////////////////

namespace Vital::System::Discord {
    // Managers //
    extern bool start(long long applicationID);
    extern bool stop();
    extern bool update();

    // APIs //
    extern bool isConnected();
    extern bool setActivity(
        const std::string& state,
        const std::string& details,
        const std::string& largeImage = "",
        const std::string& largeText = "",
        const std::string& smallImage = "",
        const std::string& smallText = ""
    );
    extern bool clearActivity();
}
