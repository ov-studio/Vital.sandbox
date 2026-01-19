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


#include <cstdint>

////////////////////////////
// Vital: System: Discord //
////////////////////////////

namespace Vital::System::Discord {
    struct ActivityData {
        std::string state;
        std::string details;
        
        // Assets
        std::string largeImageKey;
        std::string largeImageText;
        std::string smallImageKey;
        std::string smallImageText;

        // Timestamps (0 = ignored)
        int64_t startTimestamp = 0;
        int64_t endTimestamp = 0;
    };

    // Managers //
    extern bool start();
    extern bool stop();

    // APIs //
    extern bool isConnected();
    extern bool setActivity(const ActivityData& data);

    // Partial Updates
    extern bool updateState(const std::string& state);
    extern bool updateDetails(const std::string& details);
    extern bool updateLargeAsset(const std::string& key, const std::string& text = "");
    extern bool updateSmallAsset(const std::string& key, const std::string& text = "");
    extern bool updateTimestamps(int64_t start, int64_t end);
}
#endif