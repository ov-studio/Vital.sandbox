/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: config.h
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
#include <godot_cpp/classes/os.hpp>
#include <filesystem>
#include <vector>


//###############################
// Vital: Tool: Config //
//###############################

namespace Vital::Tool {
    class Config {
        private:
            YAML::Node root;
            bool loaded = false;
            std::string config_path;

            // Cache for template-based access
            template<typename T>
            T get_value(const std::string& section, const std::string& key, T default_value) {
                if (!loaded) return default_value;
                try {
                    if (root[section] && root[section][key]) {
                        return root[section].as<T>();
                    }
                } catch (const YAML::Exception&) {}
                return default_value;
            }

        public:
            Config() = default;
            ~Config() = default;

            // Load configuration from file
            // Returns true if successful, false if file not found or parse error
            bool load(const std::string& path) {
                config_path = path;
                
                // Try multiple path strategies
                std::vector<std::string> try_paths;
                
                // Get base directory (where executable is)
                godot::String base_dir = godot::OS::get_singleton()->get_executable_path().get_base_dir();
                std::string base_dir_str = std::string(base_dir.utf8().get_data());
                
                // Try relative to current working directory first (most common)
                try_paths.push_back(path);
                
                // Try relative to executable directory
                try_paths.push_back(base_dir_str + "/" + path);
                
                // Try going up from executable directory
                try_paths.push_back(base_dir_str + "/../" + path);
                
                // Also try some common relative locations
                try_paths.push_back(base_dir_str + "/../../.dist/debug/server/config.yaml");
                try_paths.push_back(base_dir_str + "/../../../.dist/debug/server/config.yaml");
                
                for (const auto& try_path : try_paths) {
                    std::filesystem::path fs_path(try_path);
                    if (std::filesystem::exists(fs_path)) {
                        try {
                            root = YAML::LoadFile(try_path);
                            loaded = true;
                            Vital::print("sbox", "Config: Loaded from '", try_path.c_str(), "'");
                            return true;
                        } catch (const YAML::Exception& e) {
                            Vital::print("error", "Config: Failed to parse '", try_path.c_str(), "' - ", e.what());
                            // Continue trying other paths even if one fails to parse
                        }
                    }
                }
                
                // None of the paths worked
                Vital::print("warn", "Config: File not found - tried ", try_paths.size(), " locations");
                return false;
            }

            // Reload configuration from file
            bool reload() {
                if (config_path.empty()) return false;
                return load(config_path);
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

            bool get_auto_reconnect() {
                if (!loaded || !root["network"] || !root["network"]["auto_reconnect"]) return true;
                return root["network"]["auto_reconnect"].as<bool>();
            }

            int get_reconnect_max_attempts() {
                if (!loaded || !root["network"] || !root["network"]["reconnect_max_attempts"]) return 5;
                return root["network"]["reconnect_max_attempts"].as<int>();
            }

            float get_reconnect_delay() {
                if (!loaded || !root["network"] || !root["network"]["reconnect_delay"]) return 3.0f;
                return root["network"]["reconnect_delay"].as<float>();
            }

            //========================================//
            // HTTP Server Configuration
            //========================================//

            int get_http_port() {
                if (!loaded || !root["http"] || !root["http"]["port"]) return 7778;
                return root["http"]["port"].as<int>();
            }

            std::string get_http_base_directory() {
                if (!loaded || !root["http"] || !root["http"]["base_directory"]) return "";
                return root["http"]["base_directory"].as<std::string>();
            }

            bool get_http_cors_enabled() {
                if (!loaded || !root["http"] || !root["http"]["enable_cors"]) return true;
                return root["http"]["enable_cors"].as<bool>();
            }

            int get_http_max_file_size_mb() {
                if (!loaded || !root["http"] || !root["http"]["max_file_size_mb"]) return 0;
                return root["http"]["max_file_size_mb"].as<int>();
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
                // Replace placeholders
                size_t pos;
                while ((pos = state.find("{server_name}")) != std::string::npos) {
                    state.replace(pos, 13, get_server_name());
                }
                while ((pos = state.find("{player_count}")) != std::string::npos) {
                    state.replace(pos, 15, "0"); // Will be updated dynamically
                }
                while ((pos = state.find("{max_players}")) != std::string::npos) {
                    state.replace(pos, 14, std::to_string(get_max_clients()));
                }
                return state;
            }

            std::string get_discord_details() {
                if (!loaded || !root["discord"] || !root["discord"]["details"]) return "{player_count}/{max_players} players";
                std::string details = root["discord"]["details"].as<std::string>();
                // Replace placeholders
                size_t pos;
                while ((pos = details.find("{player_count}")) != std::string::npos) {
                    details.replace(pos, 15, "0"); // Will be updated dynamically
                }
                while ((pos = details.find("{max_players}")) != std::string::npos) {
                    details.replace(pos, 14, std::to_string(get_max_clients()));
                }
                return details;
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

            std::string get_steam_group() {
                if (!loaded || !root["social"] || !root["social"]["steam_group"]) return "";
                return root["social"]["steam_group"].as<std::string>();
            }

            int get_recent_players_max() {
                if (!loaded || !root["social"] || !root["social"]["recent_players_max"]) return 10;
                return root["social"]["recent_players_max"].as<int>();
            }

            //========================================//
            // Logging Configuration
            //========================================//

            bool get_console_enabled() {
                if (!loaded || !root["logging"] || !root["logging"]["console_enabled"]) return true;
                return root["logging"]["console_enabled"].as<bool>();
            }

            std::string get_log_file_path() {
                if (!loaded || !root["logging"] || !root["logging"]["file_path"]) return "";
                return root["logging"]["file_path"].as<std::string>();
            }

            std::string get_log_level() {
                if (!loaded || !root["logging"] || !root["logging"]["level"]) return "info";
                return root["logging"]["level"].as<std::string>();
            }

            bool get_timestamps_enabled() {
                if (!loaded || !root["logging"] || !root["logging"]["timestamps"]) return true;
                return root["logging"]["timestamps"].as<bool>();
            }

            //========================================//
            // Advanced Settings
            //========================================//

            bool get_compress_packets() {
                if (!loaded || !root["advanced"] || !root["advanced"]["compress_packets"]) return true;
                return root["advanced"]["compress_packets"].as<bool>();
            }

            float get_sync_interval() {
                if (!loaded || !root["advanced"] || !root["advanced"]["sync_interval"]) return 0.05f;
                return root["advanced"]["sync_interval"].as<float>();
            }

            bool get_profiling_enabled() {
                if (!loaded || !root["advanced"] || !root["advanced"]["profiling"]) return false;
                return root["advanced"]["profiling"].as<bool>();
            }

            int get_tick_rate() {
                if (!loaded || !root["advanced"] || !root["advanced"]["tick_rate"]) return 60;
                return root["advanced"]["tick_rate"].as<int>();
            }

            //========================================//
            // Utility Methods
            //========================================//

            // Get any string value by section and key (for custom extensions)
            std::string get_string(const std::string& section, const std::string& key, const std::string& default_value = "") {
                if (!loaded || !root[section] || !root[section][key]) return default_value;
                return root[section][key].as<std::string>();
            }

            // Get any integer value by section and key
            int get_int(const std::string& section, const std::string& key, int default_value = 0) {
                if (!loaded || !root[section] || !root[section][key]) return default_value;
                return root[section][key].as<int>();
            }

            // Get any float value by section and key
            float get_float(const std::string& section, const std::string& key, float default_value = 0.0f) {
                if (!loaded || !root[section] || !root[section][key]) return default_value;
                return root[section][key].as<float>();
            }

            // Get any bool value by section and key
            bool get_bool(const std::string& section, const std::string& key, bool default_value = false) {
                if (!loaded || !root[section] || !root[section][key]) return default_value;
                return root[section][key].as<bool>();
            }

            // Check if a section exists
            bool has_section(const std::string& section) {
                return loaded && root[section];
            }

            // Check if a key exists in a section
            bool has_key(const std::string& section, const std::string& key) {
                return loaded && root[section] && root[section][key];
            }
    };
}
