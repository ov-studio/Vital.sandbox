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


/////////////////////////////
// Vital: System: Discord //
/////////////////////////////

namespace Vital::System {
    class Discord;
    class Discord {
        public:
            struct Activity {
                std::string state;
                std::string details;
                std::string largeimage_key = "";
                std::string largeimage_text = "";
                std::string smallimage_key = "";
                std::string smallimage_text = "";
                int64_t timestamp_start = 0;
                int64_t timestamp_end = 0;
            };
        protected:
            inline static Discord* singleton = nullptr;
            uint64_t default_application_id;
            Activity default_activity = {};
        private:
            uint64_t application_id;
            Activity activity = {};
            std::shared_ptr<discordpp::Client> client;
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
            void authorize(const godot::String& token_directory, const godot::String& token_file, bool force_reauth = false);
            bool set_application_id(uint64_t id, bool authenticate = false, bool force_reauth = false);
            uint64_t get_user_id();
            std::string get_username();
            bool set_activity(const Activity& data);
            bool reset_activity();


            // Updaters //
            bool update_state(const std::string& state);
            bool update_details(const std::string& details);
            bool update_largeimage(const std::string& key, const std::string& text = "");
            bool update_smallimage(const std::string& key, const std::string& text = "");
            bool update_timestamps(int64_t start_at, int64_t end_at = 0);
    };
}
#endif