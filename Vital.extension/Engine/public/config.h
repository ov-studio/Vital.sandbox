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
#include <godot_cpp/classes/os.hpp>
#include <filesystem>
#include <vector>


//###############################
// Vital: Engine: Config //
//###############################

namespace Vital::Engine {
    class Config {
        private:
            YAML::Node root;
            bool loaded = false;
            std::string config_path;

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
