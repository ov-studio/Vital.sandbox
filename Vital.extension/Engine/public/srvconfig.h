/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: srvconfig.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 28/03/2026
     Desc: Server Configuration Manager
----------------------------------------------------------------*/


//############
// Imports //
//############

#pragma once
#include <Vital.sandbox/Tool/yaml.h>


//////////////////////////////////
// Vital: Engine: ServerConfig //
//////////////////////////////////

namespace Vital::Engine {
    class SrvConfig {
        private:
            Vital::Tool::YAML yaml;
            bool loaded = false;

            std::string get_str(const char* section, const char* key, const std::string& fallback = "") const {
                if (!loaded || !yaml.has(section)) return fallback;
                return Vital::Tool::YAML::get_str(yaml.get_root()[section], key, fallback);
            }

            int get_int(const char* section, const char* key, int fallback = 0) const {
                if (!loaded || !yaml.has(section)) return fallback;
                return Vital::Tool::YAML::get_int(yaml.get_root()[section], key, fallback);
            }

            bool get_bool(const char* section, const char* key, bool fallback = false) const {
                if (!loaded || !yaml.has(section)) return fallback;
                return Vital::Tool::YAML::get_bool(yaml.get_root()[section], key, fallback);
            }

        public:
            SrvConfig() = default;
            ~SrvConfig() = default;

            bool load() {
                const std::string config_path = "config.yaml";
                if (!Vital::Tool::File::exists(get_directory(), config_path)) {
                    Vital::print("warn", "SrvConfig: File not found - '", config_path.c_str(), "'");
                    return false;
                }
                const std::string content = Vital::Tool::File::read_text(get_directory(), config_path);
                try {
                    yaml.parse(content);
                }
                catch (const std::exception& e) {
                    Vital::print("error", "SrvConfig: Malformed YAML — ", e.what());
                    return false;
                }
                loaded = true;
                Vital::print("sbox", "SrvConfig: Loaded from '", config_path.c_str(), "'");
                return true;
            }

            bool is_loaded() const {
                return loaded;
            }


            // Identity //
            std::string get_server_name() const { return get_str("server", "name", "Vital Sandbox Server"); }
            std::string get_server_version() const { return get_str("server", "version", "1.0.0"); }
            std::string get_server_description() const { return get_str("server", "description", ""); }


            // Network //
            int get_network_port() const { return get_int("network", "port", 7777); }
            int get_max_clients() const { return get_int("network", "max_clients", 32); }
            int get_http_port() const { return get_int("http", "port", 7778); }


            // Discord //
            bool get_discord_enabled() const { return get_bool("discord", "enabled", false); }
            std::string get_discord_state() const { return get_str("discord", "state", "Playing on {server_name}"); }
            std::string get_discord_details() const { return get_str("discord", "details", "{player_count}/{max_players} players"); }
            std::string get_discord_large_image_key() const { return get_str("discord", "large_image_key",  ""); }
            std::string get_discord_large_image_text() const { return get_str("discord", "large_image_text", ""); }
            std::string get_discord_small_image_key() const { return get_str("discord", "small_image_key",  ""); }
            std::string get_discord_small_image_text() const { return get_str("discord", "small_image_text", ""); }

            uint64_t get_discord_application_id() const {
                const std::string id_str = get_str("discord", "application_id", "");
                try { return std::stoull(id_str); } catch (...) { return 0; }
            }


            // Social //
            std::string get_discord_invite() const { return get_str("social", "discord_invite", ""); }
            std::string get_website() const { return get_str("social", "website", ""); }
    };
}