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
    

    // APIs //
    void Discord::update() {
        discordpp::Activity client_activity;
        discordpp::ActivityAssets client_assets;
        discordpp::ActivityTimestamps client_timestamps;
        client_activity.SetType(discordpp::ActivityTypes::Playing);
        client_activity.SetState(activity.state);
        client_activity.SetDetails(activity.details);
        if (!activity.largeImageKey.empty()) {
            client_assets.SetLargeImage(activity.largeImageKey);
            client_assets.SetLargeText(activity.largeImageText);
        }
        if (!activity.smallImageKey.empty()) {
            client_assets.SetSmallImage(activity.smallImageKey);
            client_assets.SetSmallText(activity.smallImageText);
        }
        client_activity.SetAssets(client_assets);
        if (activity.startTimestamp > 0) client_timestamps.SetStart(activity.startTimestamp);
        if (activity.endTimestamp > 0) client_timestamps.SetEnd(activity.endTimestamp);
        client_activity.SetTimestamps(client_timestamps);
        client -> UpdateRichPresence(client_activity, [](const discordpp::ClientResult &result) {
            if (!result.Successful()) {
                // TO DO: std::cerr << "Rich Presence update failed\n";
            }
        });
    }

    void Discord::process() {
        discordpp::RunCallbacks();
    }

    bool Discord::is_connected() {
        return !!client;
    }

    bool Discord::set_activity(const Activity& data) {
        activity = data;
        update();
        return true;
    }

    bool Discord::reset_activity() {
        activity = {};
        client -> ClearRichPresence();
        return true;
    }

    bool Discord::update_state(const std::string& state) {
        activity.state = state;
        update();
        return true;
    }

    bool Discord::update_details(const std::string& details) {
        activity.details = details;
        update();
        return true;
    }

    bool Discord::update_largeimage(const std::string& key, const std::string& text) {
        activity.largeImageKey = key;
        activity.largeImageText = text;
        update();
        return true;
    }

    bool Discord::update_smallimage(const std::string& key, const std::string& text) {
        activity.smallImageKey = key;
        activity.smallImageText = text;
        update();
        return true;
    }

    bool Discord::update_timestamps(int64_t start_at, int64_t end_at) {
        activity.startTimestamp = start_at;
        activity.endTimestamp = end_at;
        update();
        return true;
    }
}
#endif
