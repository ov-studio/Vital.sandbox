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
#define DISCORDPP_IMPLEMENTATION
#include <Vital.sandbox/System/public/discord.h>


//////////////
// Constants //
//////////////

const uint64_t APPLICATION_ID = 1461425342722998474;


////////////////////////////
// Vital: System: Discord //
////////////////////////////

namespace Vital::System {
    // Instantiators //
    Discord::Discord() {
        client = std::make_shared<discordpp::Client>();
        client -> SetApplicationId(APPLICATION_ID);
        client -> Connect();
    }

    Discord::~Discord() {
        client.reset();
        activity = {};
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

    void Discord::update_singleton() {
        discordpp::Activity client_activity;
        client_activity.SetType(discordpp::ActivityTypes::Playing);
        client_activity.SetState(activity.state);
        client_activity.SetDetails(activity.details);
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
            client_activity.SetAssets(assets);
        }
        if (activity.startTimestamp > 0 || activity.endTimestamp > 0) {
            discordpp::ActivityTimestamps client_timestamps;
            if (activity.startTimestamp > 0) client_timestamps.SetStart(activity.startTimestamp);
            if (activity.endTimestamp > 0) client_timestamps.SetEnd(activity.endTimestamp);
            client_activity.SetTimestamps(client_timestamps);
        }
        client -> UpdateRichPresence(client_activity, [](const discordpp::ClientResult &result) {
            if (!result.Successful()) {
                // TO DO: std::cerr << "Rich Presence update_singleton failed\n";
            }
        });
    }
    

    // Managers //
    void Discord::tick() {
        discordpp::RunCallbacks();
    }


    // APIs //
    bool Discord::is_connected() {
        return !!client;
    }

    bool Discord::set_activity(const Activity& data) {
        activity = data;
        update_singleton();
        return true;
    }

    bool Discord::reset_activity() {
        activity = {};
        client -> ClearRichPresence();
        return true;
    }

    bool Discord::update_state(const std::string& state) {
        activity.state = state;
        update_singleton();
        return true;
    }

    bool Discord::update_details(const std::string& details) {
        activity.details = details;
        update_singleton();
        return true;
    }

    bool Discord::update_largeimage(const std::string& key, const std::string& text) {
        activity.largeImageKey = key;
        activity.largeImageText = text;
        update_singleton();
        return true;
    }

    bool Discord::update_smallimage(const std::string& key, const std::string& text) {
        activity.smallImageKey = key;
        activity.smallImageText = text;
        update_singleton();
        return true;
    }

    bool Discord::update_timestamps(int64_t start, int64_t end) {
        activity.startTimestamp = start;
        activity.endTimestamp = end;
        update_singleton();
        return true;
    }
}
#endif
