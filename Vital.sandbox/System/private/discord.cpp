/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: discord.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene, Ianec
     DOC: 15/01/2026
     Desc: Discord Rich Presence System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/System/public/discord.h>
#include <discord-sdk/include/discord.h>


////////////////////////////
// Vital: System: Discord //
////////////////////////////

namespace Vital::System::Discord {
    discord::Core* core = nullptr;
    bool connected = false;

    // Managers //
    bool start(long long applicationID) {
        if (core) return false;
        auto result = discord::Core::Create(applicationID, DiscordCreateFlags_NoRequireDiscord, &core);
        if (result != discord::Result::Ok) {
            core = nullptr;
            return false;
        }
        connected = true;
        return true;
    }

    bool stop() {
        if (!core) return false;
        delete core;
        core = nullptr;
        connected = false;
        return true;
    }

    bool update() {
        if (!core) return false;
        auto result = core->RunCallbacks();
        if (result != discord::Result::Ok) {
            connected = false;
            return false;
        }
        return true;
    }

    // APIs //
    bool isConnected() { return connected && core; }

    bool setActivity(
        const std::string& state,
        const std::string& details,
        const std::string& largeImage,
        const std::string& largeText,
        const std::string& smallImage,
        const std::string& smallText
    ) {
        if (!core) return false;
        discord::Activity activity{};
        activity.SetState(state.c_str());
        activity.SetDetails(details.c_str());
        if (!largeImage.empty()) activity.GetAssets().SetLargeImage(largeImage.c_str());
        if (!largeText.empty()) activity.GetAssets().SetLargeText(largeText.c_str());
        if (!smallImage.empty()) activity.GetAssets().SetSmallImage(smallImage.c_str());
        if (!smallText.empty()) activity.GetAssets().SetSmallText(smallText.c_str());
        core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
            // callback when activity is updated
        });
        return true;
    }

    bool clearActivity() {
        if (!core) return false;
        core->ActivityManager().ClearActivity([](discord::Result result) {
            // callback when activity is cleared
        });
        return true;
    }
}
