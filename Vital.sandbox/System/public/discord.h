/*----------------------------------------------------------------
    Resource: Vital.sandbox
    Script: System: discord.h
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
#include <Vital.sandbox/Tool/index.h>
#include <discord-sdk/include/discordpp.h>


////////////////////////////
// Vital: System: Discord //
////////////////////////////

namespace Vital::System {
    class Discord;
    class Discord {
        public:
            struct Activity {
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
        protected:
            inline static Discord* singleton = nullptr;
        private:
            std::shared_ptr<discordpp::Client> client;
            Activity activity = {};
        public:
            // Instantiators //
            Discord();
            ~Discord();

        
            // Utils //
            static Discord* get_singleton();
            static void free_singleton();


            // Managers //
            bool stop();
            void tick();


            // APIs //
            bool isConnected();
            bool setActivity(const Activity& data);
            bool clearActivity();


            // Partial Updates //
            bool updateState(const std::string& state);
            bool updateDetails(const std::string& details);
            bool updateLargeImage(const std::string& key, const std::string& text = "");
            bool updateSmallImage(const std::string& key, const std::string& text = "");
            bool updateTimestamps(int64_t start, int64_t end);
            
            void pushUpdate();
    };
}
#endif
