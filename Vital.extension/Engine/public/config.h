/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: config.h
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


////////////////////////////
// Vital: Engine: Config //
////////////////////////////

namespace Vital::Engine {
    // TODO: Improve

    class Config {
        private:
            Vital::Tool::YAML yaml;
            bool loaded = false;

        public:
            Config() = default;
            ~Config() = default;

            bool load() {
                const std::string config_path = "config.yaml";
                if (!Vital::Tool::File::exists(get_directory(), config_path)) {
                    Vital::print("warn", "Config: File not found - '", config_path.c_str(), "'");
                    return false;
                }
                const std::string content = Vital::Tool::File::read_text(get_directory(), config_path);
                try {
                    yaml.parse(content);
                }
                catch (const std::exception& e) {
                    Vital::print("error", "Config: Malformed YAML — ", e.what());
                    return false;
                }
                loaded = true;
                Vital::print("sbox", "Config: Loaded from '", config_path.c_str(), "'");
                return true;
            }

            bool is_loaded() const { 
                return loaded;
            }


            //========================================//
            // Server Identity
            //========================================//

            std::string get_server_name() {
                if (!loaded || !yaml.has("server")) return "Vital Sandbox Server";
                return Vital::Tool::YAML::get_str(yaml.get_root()["server"], "name", "Vital Sandbox Server");
            }

            std::string get_server_version() {
                if (!loaded || !yaml.has("server")) return "1.0.0";
                return Vital::Tool::YAML::get_str(yaml.get_root()["server"], "version", "1.0.0");
            }

            std::string get_server_description() {
                if (!loaded || !yaml.has("server")) return "";
                return Vital::Tool::YAML::get_str(yaml.get_root()["server"], "description", "");
            }


            //========================================//
            // Network Configuration
            //========================================//

            int get_network_port() {
                if (!loaded || !yaml.has("network")) return 7777;
                return Vital::Tool::YAML::get_int(yaml.get_root()["network"], "port", 7777);
            }

            int get_max_clients() {
                if (!loaded || !yaml.has("network")) return 32;
                return Vital::Tool::YAML::get_int(yaml.get_root()["network"], "max_clients", 32);
            }


            //========================================//
            // HTTP Server Configuration
            //========================================//

            int get_http_port() {
                if (!loaded || !yaml.has("http")) return 7778;
                return Vital::Tool::YAML::get_int(yaml.get_root()["http"], "port", 7778);
            }


            //========================================//
            // Discord Integration
            //========================================//

            bool get_discord_enabled() {
                if (!loaded || !yaml.has("discord")) return false;
                return Vital::Tool::YAML::get_bool(yaml.get_root()["discord"], "enabled", false);
            }

            uint64_t get_discord_application_id() {
                if (!loaded || !yaml.has("discord")) return 0;
                const std::string id_str = Vital::Tool::YAML::get_str(yaml.get_root()["discord"], "application_id", "");
                try {
                    return std::stoull(id_str);
                }
                catch (...) { return 0; }
            }

            std::string get_discord_state() {
                if (!loaded || !yaml.has("discord")) return "Playing on {server_name}";
                return Vital::Tool::YAML::get_str(yaml.get_root()["discord"], "state", "Playing on {server_name}");
            }

            std::string get_discord_details() {
                if (!loaded || !yaml.has("discord")) return "{player_count}/{max_players} players";
                return Vital::Tool::YAML::get_str(yaml.get_root()["discord"], "details", "{player_count}/{max_players} players");
            }

            std::string get_discord_large_image_key() {
                if (!loaded || !yaml.has("discord")) return "";
                return Vital::Tool::YAML::get_str(yaml.get_root()["discord"], "large_image_key", "");
            }

            std::string get_discord_large_image_text() {
                if (!loaded || !yaml.has("discord")) return "";
                return Vital::Tool::YAML::get_str(yaml.get_root()["discord"], "large_image_text", "");
            }

            std::string get_discord_small_image_key() {
                if (!loaded || !yaml.has("discord")) return "";
                return Vital::Tool::YAML::get_str(yaml.get_root()["discord"], "small_image_key", "");
            }

            std::string get_discord_small_image_text() {
                if (!loaded || !yaml.has("discord")) return "";
                return Vital::Tool::YAML::get_str(yaml.get_root()["discord"], "small_image_text", "");
            }


            //========================================//
            // Social Links
            //========================================//

            std::string get_discord_invite() {
                if (!loaded || !yaml.has("social")) return "";
                return Vital::Tool::YAML::get_str(yaml.get_root()["social"], "discord_invite", "");
            }

            std::string get_website() {
                if (!loaded || !yaml.has("social")) return "";
                return Vital::Tool::YAML::get_str(yaml.get_root()["social"], "website", "");
            }
    };
}