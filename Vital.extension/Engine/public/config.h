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
#include <yaml-cpp/yaml.h>


//###############################
// Vital: Engine: Config //
//###############################

// TODO: Improve
namespace Vital::Engine {
    class Config {
        private:
            YAML::Node root;
            bool loaded = false;
        public:
            Config() = default;
            ~Config() = default;

            // Load configuration from file
            // Returns true if successful, false if file not found
            bool load() {
                std::string config_path = "config.yaml";
                
                if (!Vital::Tool::File::exists(get_directory(), config_path)) {
                    Vital::print("warn", "Config: File not found - '", config_path.c_str(), "'");
                    return false;
                }
                
                const std::string content = Vital::Tool::File::read_text(get_directory(), config_path);
                root = YAML::Load(content);
                loaded = true;
                Vital::print("sbox", "Config: Loaded from '", config_path.c_str(), "'");
                return true;
            }

            // Check if config is loaded
            bool is_loaded() const { return loaded; }

            //========================================//
            // Server Identity
            //========================================//

            std::string get_server_name() {
                if (!loaded || !root["server"] || !root["server"]["name"]) return "Vital Sandbox Server";
                return root["server"]["name"].as<std::string>();
            }

            std::string get_server_version() {
                if (!loaded || !root["server"] || !root["server"]["version"]) return "1.0.0";
                return root["server"]["version"].as<std::string>();
            }

            std::string get_server_description() {
                if (!loaded || !root["server"] || !root["server"]["description"]) return "";
                return root["server"]["description"].as<std::string>();
            }

            //========================================//
            // Network Configuration
            //========================================//

            int get_network_port() {
                if (!loaded || !root["network"] || !root["network"]["port"]) return 7777;
                return root["network"]["port"].as<int>();
            }

            int get_max_clients() {
                if (!loaded || !root["network"] || !root["network"]["max_clients"]) return 32;
                return root["network"]["max_clients"].as<int>();
            }

            //========================================//
            // HTTP Server Configuration
            //========================================//

            int get_http_port() {
                if (!loaded || !root["http"] || !root["http"]["port"]) return 7778;
                return root["http"]["port"].as<int>();
            }

            //========================================//
            // Discord Integration
            //========================================//

            bool get_discord_enabled() {
                if (!loaded || !root["discord"] || !root["discord"]["enabled"]) return false;
                return root["discord"]["enabled"].as<bool>();
            }

            uint64_t get_discord_application_id() {
                if (!loaded || !root["discord"] || !root["discord"]["application_id"]) return 0;
                std::string id_str = root["discord"]["application_id"].as<std::string>();
                try {
                    return std::stoull(id_str);
                } catch (...) {
                    return 0;
                }
            }

            std::string get_discord_state() {
                if (!loaded || !root["discord"] || !root["discord"]["state"]) return "Playing on {server_name}";
                std::string state = root["discord"]["state"].as<std::string>();
                return state;
            }

            std::string get_discord_details() {
                if (!loaded || !root["discord"] || !root["discord"]["details"]) return "{player_count}/{max_players} players";
                return root["discord"]["details"].as<std::string>();
            }

            std::string get_discord_large_image_key() {
                if (!loaded || !root["discord"] || !root["discord"]["large_image_key"]) return "";
                return root["discord"]["large_image_key"].as<std::string>();
            }

            std::string get_discord_large_image_text() {
                if (!loaded || !root["discord"] || !root["discord"]["large_image_text"]) return "";
                return root["discord"]["large_image_text"].as<std::string>();
            }

            std::string get_discord_small_image_key() {
                if (!loaded || !root["discord"] || !root["discord"]["small_image_key"]) return "";
                return root["discord"]["small_image_key"].as<std::string>();
            }

            std::string get_discord_small_image_text() {
                if (!loaded || !root["discord"] || !root["discord"]["small_image_text"]) return "";
                return root["discord"]["small_image_text"].as<std::string>();
            }

            //========================================//
            // Social Links
            //========================================//

            std::string get_discord_invite() {
                if (!loaded || !root["social"] || !root["social"]["discord_invite"]) return "";
                return root["social"]["discord_invite"].as<std::string>();
            }

            std::string get_website() {
                if (!loaded || !root["social"] || !root["social"]["website"]) return "";
                return root["social"]["website"].as<std::string>();
            }
    };
}
