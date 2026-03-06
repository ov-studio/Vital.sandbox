/*----------------------------------------------------------------
    Resource: Vital.sandbox
    Script: System: private: discord.cpp
    Author: ov-studio
    Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene, Ianec
    DOC: 15/01/2026
    Desc: Discord Rich Presence System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if defined(Vital_SDK_Client)
#pragma once
#include <Vital.sandbox/System/public/discord.h>


//////////////
// Constants //
//////////////

const uint64_t APPLICATION_ID = 1461425342722998474;


////////////////////////////
// Vital: System: Discord //
////////////////////////////

namespace Vital::System {

    Discord::Discord() {
        client = std::make_shared<discordpp::Client>();
        client -> SetApplicationId(APPLICATION_ID);
        client -> Connect();
    }

    Discord::~Discord() {
        client.reset();
        activity = {};
    }

    void Discord::pushUpdate() {
        if (!client) return;

        discordpp::Activity activity;
        activity.SetType(discordpp::ActivityTypes::Playing);
        activity.SetState(activity.state);
        activity.SetDetails(activity.details);

        if (!activity.largeImageKey.empty() || !activity.smallImageKey.empty()) {
            discordpp::ActivityAssets assets;
            if (!activity.largeImageKey.empty()) {
                assets.SetLargeImage(activity.largeImageKey);
                assets.SetLargeText(activity.largeImageText);
            }
            if (!activity.smallImageKey.empty()) {
                assets.SetSmallImage(activity.smallImageKey);
                assets.SetSmallText(activity.smallImageText);
            }
            activity.SetAssets(assets);
        }

        if (activity.startTimestamp > 0 || activity.endTimestamp > 0) {
            discordpp::ActivityTimestamps timestamps;
            if (activity.startTimestamp > 0) timestamps.SetStart(activity.startTimestamp);
            if (activity.endTimestamp > 0) timestamps.SetEnd(activity.endTimestamp);
            activity.SetTimestamps(timestamps);
        }

        client -> UpdateRichPresence(activity, [](const discordpp::ClientResult &result) {
            if (!result.Successful()) {
                // TO DO: std::cerr << "Rich Presence update failed\n";
            }
        });
    }


    // Utils //
    Discord* Discord::get_singleton() {
        if (!singleton) singleton = new Discord();
        return singleton;
    }

    void Discord::free_singleton() {
        if (!singleton) return;
        delete singleton;
        singleton = nullptr;
    }


    // Managers //
    void Discord::tick() {
        if (!client) return;
        discordpp::RunCallbacks();
    }


    // APIs //
    bool Discord::isConnected() { return !!client; }

    bool Discord::setActivity(const Activity& data) {
        if (!client) return false;
        activity = data;
        pushUpdate();
        return true;
    }

    bool Discord::clearActivity() {
        if (!client) return false;
        activity = {};
        client -> ClearRichPresence();
        return true;
    }

    bool Discord::updateState(const std::string& state) {
        if (!client) return false;
        activity.state = state;
        pushUpdate();
        return true;
    }

    bool Discord::updateDetails(const std::string& details) {
        if (!client) return false;
        activity.details = details;
        pushUpdate();
        return true;
    }

    bool Discord::updateLargeImage(const std::string& key, const std::string& text) {
        if (!client) return false;
        activity.largeImageKey = key;
        activity.largeImageText = text;
        pushUpdate();
        return true;
    }

    bool Discord::updateSmallImage(const std::string& key, const std::string& text) {
        if (!client) return false;
        activity.smallImageKey = key;
        activity.smallImageText = text;
        pushUpdate();
        return true;
    }

    bool Discord::updateTimestamps(int64_t start, int64_t end) {
        if (!client) return false;
        activity.startTimestamp = start;
        activity.endTimestamp = end;
        pushUpdate();
        return true;
    }
}
#endif
