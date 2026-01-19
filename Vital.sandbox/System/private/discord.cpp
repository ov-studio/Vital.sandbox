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

        if (!currentData.largeImageKey.empty() || !currentData.smallImageKey.empty()) {
            discordpp::ActivityAssets assets;
            if (!currentData.largeImageKey.empty()) {
                assets.SetLargeImage(currentData.largeImageKey);
                assets.SetLargeText(currentData.largeImageText);
            }
            if (!currentData.smallImageKey.empty()) {
                assets.SetSmallImage(currentData.smallImageKey);
                assets.SetSmallText(currentData.smallImageText);
            }
            activity.SetAssets(assets);
        }

        if (currentData.startTimestamp > 0 || currentData.endTimestamp > 0) {
            discordpp::ActivityTimestamps timestamps;
            if (currentData.startTimestamp > 0) {
                timestamps.SetStart(currentData.startTimestamp);
            }
            if (currentData.endTimestamp > 0) {
                timestamps.SetEnd(currentData.endTimestamp);
            }
            activity.SetTimestamps(timestamps);
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

    bool updateLargeImage(const std::string& key, const std::string& text) {
        if (!client) return false;
        currentData.largeImageKey = key;
        currentData.largeImageText = text;
        pushUpdate();
        return true;
    }

    bool updateSmallImage(const std::string& key, const std::string& text) {
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