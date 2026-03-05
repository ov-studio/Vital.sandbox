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
#include <string>
#include <cstdint>
#include <memory>

////////////////////////////
// Vital: System: Discord //
////////////////////////////

namespace Vital::System {
    class Discord {
        protected:
            inline static Discord* singleton = nullptr;
        public:
            struct ActivityData {
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

            // Utils //
            static Discord* get_singleton();
            static void free_singleton();

            // Managers //
            bool start();
            bool stop();
            void tick();

            // APIs //
            bool isConnected();
            bool setActivity(const ActivityData& data);
            bool clearActivity();

            // Partial Updates //
            bool updateState(const std::string& state);
            bool updateDetails(const std::string& details);
            bool updateLargeImage(const std::string& key, const std::string& text = "");
            bool updateSmallImage(const std::string& key, const std::string& text = "");
            bool updateTimestamps(int64_t start, int64_t end);
    };
}
#endif
