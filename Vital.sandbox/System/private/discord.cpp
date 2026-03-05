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
#define DISCORDPP_IMPLEMENTATION // this is needed, do not remove
#pragma once
#include <Vital.sandbox/System/public/discord.h>
#include <discord-sdk/include/discordpp.h>

//////////////
// Constants //
//////////////
const uint64_t APPLICATION_ID = 1461425342722998474;


////////////////////////////
// Vital: System: Discord //
////////////////////////////

namespace Vital::System {
    namespace {
        std::shared_ptr<discordpp::Client> discord_client;
        Discord::ActivityData discord_current_data;

        void pushUpdate() {
            if (!discord_client) return;

            discordpp::Activity activity;
            activity.SetType(discordpp::ActivityTypes::Playing);
            activity.SetState(discord_current_data.state);
            activity.SetDetails(discord_current_data.details);

            if (!discord_current_data.largeImageKey.empty() || !discord_current_data.smallImageKey.empty()) {
                discordpp::ActivityAssets assets;
                if (!discord_current_data.largeImageKey.empty()) {
                    assets.SetLargeImage(discord_current_data.largeImageKey);
                    assets.SetLargeText(discord_current_data.largeImageText);
                }
                if (!discord_current_data.smallImageKey.empty()) {
                    assets.SetSmallImage(discord_current_data.smallImageKey);
                    assets.SetSmallText(discord_current_data.smallImageText);
                }
                activity.SetAssets(assets);
            }

            if (discord_current_data.startTimestamp > 0 || discord_current_data.endTimestamp > 0) {
                discordpp::ActivityTimestamps timestamps;
                if (discord_current_data.startTimestamp > 0) timestamps.SetStart(discord_current_data.startTimestamp);
                if (discord_current_data.endTimestamp > 0) timestamps.SetEnd(discord_current_data.endTimestamp);
                activity.SetTimestamps(timestamps);
            }

            discord_client->UpdateRichPresence(activity, [](const discordpp::ClientResult &result) {
                if (!result.Successful()) {
                    // TO DO: std::cerr << "Rich Presence update failed\n";
                }
            });
        }
    }


    // Utils //
    Discord* Discord::get_singleton() {
        if (!singleton) singleton = new Discord();
        return singleton;
    }

    void Discord::free_singleton() {
        if (!singleton) return;
        singleton->stop();
        delete singleton;
        singleton = nullptr;
    }


    // Managers //
    bool Discord::start() {
        if (discord_client) return true;
        discord_client = std::make_shared<discordpp::Client>();
        discord_client->SetApplicationId(APPLICATION_ID);
        discord_client->Connect();
        return true;
    }

    void Discord::tick() {
        if (!discord_client) return;
        discordpp::RunCallbacks();
    }

    bool Discord::stop() {
        if (!discord_client) return false;
        discord_client.reset();
        discord_current_data = {};
        return true;
    }


    // APIs //
    bool Discord::isConnected() { return !!discord_client; }

    bool Discord::setActivity(const ActivityData& data) {
        if (!discord_client) return false;
        discord_current_data = data;
        pushUpdate();
        return true;
    }

    bool Discord::clearActivity() {
        if (!discord_client) return false;
        discord_current_data = {};
        discord_client->ClearRichPresence();
        return true;
    }

    bool Discord::updateState(const std::string& state) {
        if (!discord_client) return false;
        discord_current_data.state = state;
        pushUpdate();
        return true;
    }

    bool Discord::updateDetails(const std::string& details) {
        if (!discord_client) return false;
        discord_current_data.details = details;
        pushUpdate();
        return true;
    }

    bool Discord::updateLargeImage(const std::string& key, const std::string& text) {
        if (!discord_client) return false;
        discord_current_data.largeImageKey = key;
        discord_current_data.largeImageText = text;
        pushUpdate();
        return true;
    }

    bool Discord::updateSmallImage(const std::string& key, const std::string& text) {
        if (!discord_client) return false;
        discord_current_data.smallImageKey = key;
        discord_current_data.smallImageText = text;
        pushUpdate();
        return true;
    }

    bool Discord::updateTimestamps(int64_t start, int64_t end) {
        if (!discord_client) return false;
        discord_current_data.startTimestamp = start;
        discord_current_data.endTimestamp = end;
        pushUpdate();
        return true;
    }
}
#endif
