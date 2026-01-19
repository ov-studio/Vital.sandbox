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

#if defined(Vital_SDK_Client)
#define DISCORDPP_IMPLEMENTATION // this is needed, do not remove
#pragma once
#include <Vital.sandbox/System/public/discord.h>
#include <discord-sdk/include/discordpp.h>
#include <csignal>
#include <iostream>

//////////////
// Constants //
//////////////
const uint64_t APPLICATION_ID = 1461425342722998474;


////////////////////////////
// Vital: System: Discord //
////////////////////////////

namespace Vital::System::Discord {
    std::shared_ptr<discordpp::Client> client;
    std::atomic<bool> running = false;

    void signalHandler(int signum) {
        running.store(false);
    }

    void PerformAuthentication();

    // Managers //
    bool start() {     
        client = std::make_shared<discordpp::Client>();

        client->SetApplicationId(APPLICATION_ID);
        running = true;

        return true;
    }

    ActivityData currentData;
    void pushUpdate() {
        if (!client) return;

        discordpp::Activity activity;
        activity.SetType(discordpp::ActivityTypes::Playing);
        activity.SetState(currentData.state);
        activity.SetDetails(currentData.details);

        if (!currentData.largeImageKey.empty()) {
            activity.SetLargeImage(currentData.largeImageKey);
            activity.SetLargeText(currentData.largeImageText);
        }

        if (!currentData.smallImageKey.empty()) {
            activity.SetSmallImage(currentData.smallImageKey);
            activity.SetSmallText(currentData.smallImageText);
        }

        if (currentData.startTimestamp > 0) {
            activity.SetStartTimestamp(currentData.startTimestamp);
        }

        if (currentData.endTimestamp > 0) {
            activity.SetEndTimestamp(currentData.endTimestamp);
        }

        client->UpdateRichPresence(activity, [](const discordpp::ClientResult &result) {
            if (!result.Successful()) {
                // TO DO: std::cerr << "Rich Presence update failed\n";
            }
        });
    }

    bool setActivity(const ActivityData& data) {
        if (!client) {
            // TO DO: std::cerr << "Cannot set activity: Client is null.\n";
            return false;
        }
        currentData = data;
        pushUpdate();
        return true;
    }

    bool updateState(const std::string& state) {
        if (!client) return false;
        currentData.state = state;
        pushUpdate();
        return true;
    }

    bool updateDetails(const std::string& details) {
        if (!client) return false;
        currentData.details = details;
        pushUpdate();
        return true;
    }

    bool updateLargeAsset(const std::string& key, const std::string& text) {
        if (!client) return false;
        currentData.largeImageKey = key;
        currentData.largeImageText = text;
        pushUpdate();
        return true;
    }

    bool updateSmallAsset(const std::string& key, const std::string& text) {
        if (!client) return false;
        currentData.smallImageKey = key;
        currentData.smallImageText = text;
        pushUpdate();
        return true;
    }

    bool updateTimestamps(int64_t start, int64_t end) {
        if (!client) return false;
        currentData.startTimestamp = start;
        currentData.endTimestamp = end;
        pushUpdate();
        return true;
    }

    bool stop() {
        if (!client) return false;
        client.reset();
        running = false;
        return true;
    }

    // // APIs //
    bool isConnected() { return running && client; }
}
#endif