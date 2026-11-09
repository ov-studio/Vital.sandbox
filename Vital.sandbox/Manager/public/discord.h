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

namespace Vital::Manager {
    class Discord : public godot::Node, public Tool::Base<Discord> {
        friend class Tool::Base<Discord>;
        public:
            static constexpr const char* Name = "Discord.manager";
            static constexpr const char* NO_TEXT = "N/A";

            struct Activity {
                std::string state;
                std::string details;
                std::string small_image_key = "";
                std::string small_image_text = NO_TEXT;
                std::string large_image_key = "";
                std::string large_image_text = NO_TEXT;
                int64_t timestamp_start = 0;
                int64_t timestamp_end = 0;
            };
        private:
            static constexpr uint64_t SERVER_REFRESH_MS = 10000;

            struct InfoRequest {
                std::atomic<bool> done{false};
                std::string body;
            };

            struct Defaults {
                uint64_t application_id = 0;
                Activity activity = {};
                std::string brand_logo;
                std::string brand_name;
            };

            struct Server {
                bool joined = false;
                std::string name;
                std::string logo;
                std::string info_url;
                int players = -1;
                int max_peers = -1;
                int64_t started_at = 0;
                Activity activity = {};
                uint64_t next_refresh = 0;
                std::shared_ptr<InfoRequest> info_request;
            };

            uint64_t application_id = 0;
            Activity activity = {};
            Defaults defaults = {};
            Server server = {};
            std::shared_ptr<discordpp::Client> client;
            bool script_controlled = false;


            // Instantiators //
            Discord() = default;
            ~Discord();


            // Helpers //
            static std::string get_image_text(const std::string& text);
            Activity get_base_activity() const;
            void build_server_activity();
            void apply_application(uint64_t id, bool authenticate, bool force_reauth);
            void show(const Activity& target);
            template <typename F> bool customize(F&& fn);
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
            bool set_state(const std::string& state);
            bool set_details(const std::string& details);
            bool set_small_image(const std::string& key, const std::string& text = NO_TEXT);
            bool set_large_image(const std::string& key, const std::string& text = NO_TEXT);
            bool set_timestamps(int64_t start_at, int64_t end_at = 0);
            bool reset_application();
            bool reset_if_customized();
    };
}
#endif
