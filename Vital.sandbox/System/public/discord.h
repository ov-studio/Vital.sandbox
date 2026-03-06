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


            // APIs //
            void update();
            void process();
            bool is_connected();
            bool set_activity(const Activity& data);
            bool reset_activity();


            // Updaters //
            bool update_state(const std::string& state);
            bool update_details(const std::string& details);
            bool update_largeimage(const std::string& key, const std::string& text = "");
            bool update_smallimage(const std::string& key, const std::string& text = "");
            bool update_timestamps(int64_t start_at, int64_t end_at);
    };
}
#endif
