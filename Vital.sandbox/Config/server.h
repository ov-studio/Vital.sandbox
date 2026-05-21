/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Config: server.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 28/03/2026
     Desc: Server Configuration
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/console.h>


////////////////////////////
// Vital: Config: Server //
////////////////////////////

namespace Vital::Config {
    class Server {
        private:
            Tool::YAML yaml;
            bool loaded = false;

            std::string get_str(const char* section, const char* key, const std::string& fallback = "") const {
                if (!loaded || !yaml.has(section)) return fallback;
                return Tool::YAML::get_str(yaml.get_root()[section], key, fallback);
            }

            int get_int(const char* section, const char* key, int fallback = 0) const {
                if (!loaded || !yaml.has(section)) return fallback;
                return Tool::YAML::get_int(yaml.get_root()[section], key, fallback);
            }

            bool get_bool(const char* section, const char* key, bool fallback = false) const {
                if (!loaded || !yaml.has(section)) return fallback;
                return Tool::YAML::get_bool(yaml.get_root()[section], key, fallback);
            }
        public:
            Server() = default;
            ~Server() = default;

            bool load() {
                const std::string config_path = "config.yaml";
                bool result = [&]() -> bool {
                    if (!Tool::File::exists(Tool::get_directory(), config_path)) return false;
                    const std::string content = Tool::File::read_text(Tool::get_directory(), config_path);
                    try {
                        yaml.parse(content);
                    }
                    catch (const std::exception& e) {
                        Tool::print("error", "Server: Malformed YAML — ", e.what());
                        return false;
                    }
                    loaded = true;
                    return true;
                }();
                Tool::print(loaded ? "sbox" : "warn", fetch_info());
                return result;
            }

            bool is_loaded() const {
                return loaded;
            }

            std::string fetch_info() const {
                auto append_field = [](std::ostringstream& oss, const std::string& label, const std::string& value) {
                    oss << fmt::format("> {} — `{}`\n", label, value.empty() ? "—" : value);
                };
                std::ostringstream oss;
                oss << fmt::format("Server Config: {}\n", loaded ? "Loaded successfully" : "Using defaults");
                oss << "• Server:\n";
                append_field(oss, "Name", get_server_name());
                append_field(oss, "Version", get_server_version());
                append_field(oss, "Website", get_website());
                append_field(oss, "Discord", get_discord());
                oss << "• Network:\n";
                append_field(oss, "Port", std::to_string(get_network_port()));
                append_field(oss, "HTTP Port", std::to_string(get_http_port()));
                append_field(oss, "Max Peers", std::to_string(get_max_clients()));
                return oss.str();
            }


            // Identity //
            std::string get_server_name() const { return get_str("server", "name", "Vital Sandbox Server"); }
            std::string get_server_version() const { return get_str("server", "version", "1.0.0"); }
            std::string get_server_description() const { return get_str("server", "description", ""); }


            // Network //
            int get_network_port() const { return get_int("network", "port", 7777); }
            int get_max_clients() const { return get_int("network", "max_peers", 32); }
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
                try { return std::stoull(get_str("discord", "application_id", "")); }
                catch (...) { return 0; }
            }


            // Social //
            std::string get_discord() const { return get_str("social", "discord_invite", ""); }
            std::string get_website() const { return get_str("social", "website", ""); }
    };
}