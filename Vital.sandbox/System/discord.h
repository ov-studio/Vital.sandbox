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

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Tool/index.h>
#include <discord-sdk/include/discordpp.h>


/////////////////////////////
// Vital: System: Discord //
/////////////////////////////

namespace Vital::System {
    class Discord {
        public:
            struct Activity {
                std::string state;
                std::string details;
                std::string largeimage_key = "";
                std::string largeimage_text = "N/A";
                std::string smallimage_key = "";
                std::string smallimage_text = "N/A";
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


            // Managers //
            void process();
            void update();
            void authorize(const std::string& token_directory, const std::string& token_file, bool force_reauth = false);


            // Checkers //
            bool is_connected();


            // Setters //
            bool set_application_id(uint64_t id, bool authenticate = false, bool force_reauth = false);
            bool reset_application();
            bool set_state(const std::string& state);
            bool set_details(const std::string& details);
            bool set_largeimage(const std::string& key, const std::string& text = "N/A");
            bool set_smallimage(const std::string& key, const std::string& text = "N/A");
            bool set_timestamps(int64_t start_at, int64_t end_at = 0);


            // Getters //
            uint64_t get_userid();
            std::string get_username();
    };
}
#endif