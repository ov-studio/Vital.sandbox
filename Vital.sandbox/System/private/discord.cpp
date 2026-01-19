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

    bool setActivity(const std::string& state, const std::string& details) {
        if (!client) {
            std::cerr << "⚠️ Cannot set activity: Client is null.\n";
            return false;
        }

        discordpp::Activity activity;
        activity.SetType(discordpp::ActivityTypes::Playing);
        activity.SetState(state);
        activity.SetDetails(details);

        // Update rich presence
        client->UpdateRichPresence(activity, [](const discordpp::ClientResult &result) {
            if (result.Successful()) {
                std::cout << "Rich Presence updated successfully!\n";
            } else {
                std::cerr << "Rich Presence update failed";
            }
        });

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