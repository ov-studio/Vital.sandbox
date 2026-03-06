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
#include <Vital.sandbox/System/discord.h>
#include <Vital.extension/Engine/public/console.h>


/////////////////////////////
// Vital: System: Discord //
/////////////////////////////

namespace Vital::System {
    // Instantiators //
    Discord::Discord() {
        default_application_id = 1461425342722998474;
        default_activity = {
            "In Lobby",      // state
            "Browsing games" // details
        };
        client = std::make_shared<discordpp::Client>();
        reset_activity();
    }

    Discord::~Discord() {
        client.reset();
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
        if (!is_connected()) return;
        discordpp::Activity client_activity;
        discordpp::ActivityAssets client_assets;
        discordpp::ActivityTimestamps client_timestamps;
        client_activity.SetType(discordpp::ActivityTypes::Playing);
        client_activity.SetState(activity.state);
        client_activity.SetDetails(activity.details);
        if (!activity.largeimage_key.empty()) {
            client_assets.SetLargeImage(activity.largeimage_key);
            client_assets.SetLargeText(activity.largeimage_text);
        }
        if (!activity.smallimage_key.empty()) {
            client_assets.SetSmallImage(activity.smallimage_key);
            client_assets.SetSmallText(activity.smallimage_text);
        }
        client_activity.SetAssets(client_assets);
        if (activity.timestamp_start > 0) client_timestamps.SetStart(activity.timestamp_start);
        if (activity.timestamp_end > 0) client_timestamps.SetEnd(activity.timestamp_end);
        client_activity.SetTimestamps(client_timestamps);
        client -> UpdateRichPresence(client_activity, [](const discordpp::ClientResult& result) {
            if (!result.Successful()) {
                Vital::print("error", "[Discord] RPC update failed: " + result.ToString());
            }
        });
    }

    void Discord::process() {
        discordpp::RunCallbacks();
    }

    bool Discord::is_connected() {
        auto status = client->GetStatus();
        return status == discordpp::Client::Status::Ready || status == discordpp::Client::Status::Connected;
    }

    bool Discord::set_application_id(uint64_t id) {
        application_id = id;
        client -> SetApplicationId(application_id);
        client -> SetStatusChangedCallback([this](discordpp::Client::Status status, discordpp::Client::Error, int32_t) {
            if (status == discordpp::Client::Status::Connected || status == discordpp::Client::Status::Ready)
                update();
        });
        client -> Connect();
        return true;
    }

    uint64_t Discord::get_user_id() {
        if (!is_connected()) return 0;
        auto user = client -> GetCurrentUser();
        return user.Id();
    }

    bool Discord::set_activity(const Activity& data) {
        activity = data;
        update();
        return true;
    }

    bool Discord::reset_activity() {
        set_application_id(default_application_id);
        set_activity(default_activity);
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
        activity.largeimage_key = key;
        activity.largeimage_text = text;
        update();
        return true;
    }

    bool Discord::update_smallimage(const std::string& key, const std::string& text) {
        activity.smallimage_key = key;
        activity.smallimage_text = text;
        update();
        return true;
    }

    bool Discord::update_timestamps(int64_t start_at, int64_t end_at) {
        activity.timestamp_start = start_at;
        activity.timestamp_end = end_at;
        update();
        return true;
    }
}
#endif