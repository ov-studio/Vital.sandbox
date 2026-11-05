/*----------------------------------------------------------------
    Resource: Vital.sandbox
    Script: Manager: discord.h
    Author: ov-studio
    Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene, Ianec
    DOC: 15/01/2026
    Desc: Discord Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


//////////////////////////////
// Vital: Manager: Discord //
//////////////////////////////

// TODO: Improve
namespace Vital::Manager {
    class Discord : public godot::Node, public Tool::Base<Discord> {
        friend class Tool::Base<Discord>;
        public:
            static constexpr const char* Name = "Discord.manager";

            struct Activity {
                std::string state;
                std::string details;
                std::string small_image_key = "";
                std::string small_image_text = "N/A";
                std::string large_image_key = "";
                std::string large_image_text = "N/A";
                int64_t timestamp_start = 0;
                int64_t timestamp_end = 0;
            };
        protected:
            uint64_t default_application_id;
            Activity default_activity = {};
        private:
            static constexpr uint64_t SERVER_REFRESH_MS = 10000;

            // Result of a background /info poll, handed back to the main thread
            struct InfoPoll {
                std::atomic<bool> done{false};
                std::string body;
            };

            uint64_t application_id;
            Activity activity = {};
            std::shared_ptr<discordpp::Client> client;

            // Default presence of the server we are currently in (used when its scripts don't customize presence)
            bool in_server = false;
            bool script_controlled = false; // a server script changed application/presence: stop auto-managing it
            Activity server_activity = {};
            std::string server_name;
            std::string server_logo;
            int server_players = -1;
            int server_max_peers = -1;
            std::string server_info_url;
            int64_t server_activity_start = 0;
            uint64_t server_next_refresh = 0;
            std::shared_ptr<InfoPoll> info_poll;

            Activity get_base_activity() const;
            void build_server_activity();
            void apply_application(uint64_t id, bool authenticate, bool force_reauth);


            // Instantiators //
            Discord() = default;
            ~Discord();
        public:
            // Managers //
            void init();
            void process();
            void enter_server();
            void leave_server();
            void update();
            void authorize(const std::string& token_directory, const std::string& token_file, bool force_reauth = false);


            // Checkers //
            bool is_connected() const;


            // Getters //
            uint64_t get_userid() const;
            std::string get_username() const;


            // Setters //
            bool set_application_id(uint64_t id, bool authenticate = false, bool force_reauth = false);
            bool reset_application();
            bool reset_if_customized(); // TOOD: Improve
            bool set_state(const std::string& state);
            bool set_details(const std::string& details);
            bool set_small_image(const std::string& key, const std::string& text = "N/A");
            bool set_large_image(const std::string& key, const std::string& text = "N/A");
            bool set_timestamps(int64_t start_at, int64_t end_at = 0);
    };
}
#endif