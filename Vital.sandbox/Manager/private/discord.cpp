/*----------------------------------------------------------------
    Resource: Vital.sandbox
    Script: Manager: discord.cpp
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
#define DISCORDPP_IMPLEMENTATION
#include <Vital.sandbox/Manager/public/discord.h>
#include <Vital.sandbox/Manager/public/kit.h>
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Tool/http.h>


//////////////////////////////
// Vital: Manager: Discord //
//////////////////////////////

// TODO: Improve
namespace Vital::Manager {
    // Instantiators //
    Discord::~Discord() {
        client.reset();
    }


    // Managers //
    void Discord::init() {
        default_application_id = static_cast<uint64_t>(std::stoull(Manager::Kit::get_singleton() -> get_json_value("config/discord", "application_id").as<std::string>()));
        default_activity = {
            Manager::Kit::get_singleton() -> get_json_value("config/discord", "state").as<std::string>(),
            Manager::Kit::get_singleton() -> get_json_value("config/discord", "details").as<std::string>()
        };
        reset_application();
    }

    void Discord::process() {
        discordpp::RunCallbacks();
        if (!in_server) return;

        // Collect a finished background poll
        if (info_poll && info_poll -> done.load()) {
            auto poll = std::move(info_poll);
            info_poll.reset();
            rapidjson::Document doc;
            if (!script_controlled && !doc.Parse(poll -> body.c_str()).HasParseError() && doc.IsObject()) {
                bool changed = false;
                if (doc.HasMember("name") && doc["name"].IsString() && server_name != doc["name"].GetString()) { server_name = doc["name"].GetString(); changed = true; }
                if (doc.HasMember("logo") && doc["logo"].IsString() && server_logo != doc["logo"].GetString()) { server_logo = doc["logo"].GetString(); changed = true; }
                if (doc.HasMember("players") && doc["players"].IsInt() && server_players != doc["players"].GetInt()) { server_players = doc["players"].GetInt(); changed = true; }
                if (doc.HasMember("max_peers") && doc["max_peers"].IsInt() && server_max_peers != doc["max_peers"].GetInt()) { server_max_peers = doc["max_peers"].GetInt(); changed = true; }
                if (changed) {
                    build_server_activity();
                    activity = server_activity;
                    update();
                }
            }
        }

        // Start a new poll every few seconds (never blocks the main thread)
        if (script_controlled || info_poll || server_info_url.empty()) return;
        auto now = static_cast<uint64_t>(godot::Time::get_singleton() -> get_ticks_msec());
        if (now < server_next_refresh) return;
        server_next_refresh = now + SERVER_REFRESH_MS;
        auto poll = std::make_shared<InfoPoll>();
        info_poll = poll;
        std::thread([poll, url = server_info_url]() {
            try { poll -> body = Tool::HTTP::get(url, {}, 3); }
            catch (...) { poll -> body.clear(); }
            poll -> done = true;
        }).detach();
    }

    Discord::Activity Discord::get_base_activity() const {
        return in_server ? server_activity : default_activity;
    }

    void Discord::build_server_activity() {
        server_activity = default_activity;
        server_activity.details = server_name.empty() ? "Playing on a server" : server_name;
        if (server_players >= 0 && server_max_peers > 0) server_activity.state = fmt::format("Players: {}/{}", server_players, server_max_peers);
        else if (server_players >= 0) server_activity.state = fmt::format("Players: {}", server_players);
        else server_activity.state = "In Game";
        server_activity.timestamp_start = server_activity_start;
        // Server logo as the large image (Discord needs a public https URL, otherwise keep the default icon)
        if (server_logo.rfind("https://", 0) == 0 && server_logo.size() <= 256) {
            server_activity.large_image_key = server_logo;
            server_activity.large_image_text = server_name.empty() ? "N/A" : server_name;
        }
    }

    // Called once connected & authenticated: show the server (name, players/max) unless its scripts already took over
    void Discord::enter_server() {
        auto nm = Manager::Network::get_singleton();
        in_server = true;
        script_controlled = false;
        info_poll.reset();
        server_name = nm -> get_server_name();
        server_logo = nm -> get_server_logo_url();
        server_players = nm -> get_server_players();
        server_max_peers = nm -> get_server_max_peers();
        server_activity_start = static_cast<int64_t>(godot::Time::get_singleton() -> get_unix_time_from_system());
        server_info_url = nm -> get_server_http_port() > 0 ? fmt::format("http://{}:{}/info", nm -> get_server_ip(), nm -> get_server_http_port()) : "";
        server_next_refresh = static_cast<uint64_t>(godot::Time::get_singleton() -> get_ticks_msec()) + SERVER_REFRESH_MS;
        build_server_activity();
        if (!client) return;
        activity = server_activity;
        update();
    }

    // Left (or lost) the server: drop whatever its scripts set and go back to the lobby presence
    void Discord::leave_server() {
        const bool was_customized = application_id != default_application_id;
        in_server = false;
        script_controlled = false;
        info_poll.reset();
        server_info_url.clear();
        server_name.clear();
        server_logo.clear();
        server_players = server_max_peers = -1;
        if (was_customized) { reset_application(); return; }
        if (!client) return;
        activity = default_activity;
        update();
    }

    void Discord::update() {
        discordpp::Activity client_activity;
        discordpp::ActivityAssets client_assets;
        discordpp::ActivityTimestamps client_timestamps;
        client_activity.SetType(discordpp::ActivityTypes::Playing);
        client_activity.SetState(activity.state);
        client_activity.SetDetails(activity.details);
        if (!activity.small_image_key.empty()) {
            client_assets.SetSmallImage(activity.small_image_key);
            client_assets.SetSmallText(activity.small_image_text);
        }
        if (!activity.large_image_key.empty()) {
            client_assets.SetLargeImage(activity.large_image_key);
            client_assets.SetLargeText(activity.large_image_text);
        }
        client_activity.SetAssets(client_assets);
        if (activity.timestamp_start > 0) client_timestamps.SetStart(activity.timestamp_start);
        if (activity.timestamp_end > 0) client_timestamps.SetEnd(activity.timestamp_end);
        client_activity.SetTimestamps(client_timestamps);
        client -> UpdateRichPresence(client_activity, [this](const discordpp::ClientResult& result) {
            if (!result.Successful()) log("error", fmt::format("rpc ~ update failed | reason ~ {}", result.ToString()));
        });
    }

    void Discord::authorize(const std::string& token_directory, const std::string& token_file, bool force_reauth) {
        auto verifier = client -> CreateAuthorizationCodeVerifier();
        discordpp::AuthorizationArgs args;
        args.SetClientId(application_id);
        args.SetScopes(discordpp::Client::GetDefaultPresenceScopes());
        args.SetCodeChallenge(verifier.Challenge());
        client -> Authorize(args, [this, verifier, token_directory, token_file, force_reauth](discordpp::ClientResult result, std::string code, std::string redirectUri) {
            if (!result.Successful()) {
                if (force_reauth) {
                    log("warn", "authorization ~ rejected | action ~ retrying");
                    authorize(token_directory, token_file, force_reauth);
                }
                else {
                    log("warn", "authorization ~ rejected | action ~ presence only");
                    client -> Connect();
                }
                return;
            }
            client -> GetToken(application_id, code, verifier.Verifier(), redirectUri, [this, token_directory, token_file](discordpp::ClientResult result, std::string accessToken, std::string refreshToken, discordpp::AuthorizationTokenType tokenType, int32_t expiresIn, std::string scopes) {
                    if (!result.Successful()) { log("error", fmt::format("token ~ exchange failed | reason ~ {}", result.ToString())); return; }
                    Tool::File::write_text(token_directory, token_file, accessToken);
                    client -> UpdateToken(tokenType, accessToken, [this](discordpp::ClientResult result) {
                        if (!result.Successful()) { log("error", fmt::format("token ~ update failed | reason ~ {}", result.ToString())); return; }
                        client -> Connect();
                    });
                }
            );
        });
    }


    // Checkers //
    bool Discord::is_connected() const {
        auto status = client -> GetStatus();
        return status == discordpp::Client::Status::Ready || status == discordpp::Client::Status::Connected;
    }


    // Getters //
    uint64_t Discord::get_userid() const {
        if (!is_connected()) return 0;
        auto user = client -> GetCurrentUser();
        return user.Id();
    }

    std::string Discord::get_username() const {
        if (!is_connected()) return "";
        auto user = client -> GetCurrentUser();
        return user.Username();
    }


    // Setters //
    bool Discord::set_application_id(uint64_t id, bool authenticate, bool force_reauth) {
        script_controlled = true;
        apply_application(id, authenticate, force_reauth);
        return true;
    }

    void Discord::apply_application(uint64_t id, bool authenticate, bool force_reauth) {
        application_id = id;
        client.reset();
        client = std::make_shared<discordpp::Client>();
        activity = get_base_activity();
        client -> SetApplicationId(application_id);
        client -> SetStatusChangedCallback([this](discordpp::Client::Status status, discordpp::Client::Error, int32_t errorCode) {
            log("info", fmt::format("status ~ {} | code ~ {}", std::string(discordpp::EnumToString(status)), errorCode));
            if (status == discordpp::Client::Status::Ready || status == discordpp::Client::Status::Connected) update();
        });
        if (!authenticate) {
            client -> Connect();
            return;
        }
        auto token_directory = Tool::get_directory("data", "discord");
        auto token_file = fmt::format("{}.token", application_id);
        std::string token_value = Tool::File::exists(token_directory, token_file) ? Tool::File::read_text(token_directory, token_file) : "";
        if (!token_value.empty()) {
            client -> UpdateToken(discordpp::AuthorizationTokenType::Bearer, token_value, [this, token_directory, token_file, force_reauth](discordpp::ClientResult result) {
                if (!result.Successful()) {
                    log("warn", "token ~ cached expired | action ~ re-authenticating");
                    Tool::File::remove(token_directory, token_file);
                    authorize(token_directory, token_file, force_reauth);
                    return;
                }
                client -> Connect();
            });
            return;
        }
        log("info", "authorization ~ awaiting");
        authorize(token_directory, token_file, force_reauth);
    }

    bool Discord::set_state(const std::string& state) {
        script_controlled = true;
    activity.state = state;
        update();
        return true;
    }

    bool Discord::set_details(const std::string& details) {
        script_controlled = true;
    activity.details = details;
        update();
        return true;
    }

    bool Discord::set_small_image(const std::string& key, const std::string& text) {
        script_controlled = true;
    activity.small_image_key = key;
        activity.small_image_text = text;
        update();
        return true;
    }

    bool Discord::set_large_image(const std::string& key, const std::string& text) {
        script_controlled = true;
    activity.large_image_key = key;
        activity.large_image_text = text;
        update();
        return true;
    }

    bool Discord::set_timestamps(int64_t start_at, int64_t end_at) {
        auto now = static_cast<int64_t>(godot::Time::get_singleton() -> get_unix_time_from_system());
        script_controlled = true;
        activity.timestamp_start = now + start_at;
        activity.timestamp_end = end_at > 0 ? now + end_at : 0;
        update();
        return true;
    }

    bool Discord::reset_application() {
        script_controlled = false;
        apply_application(default_application_id, false, false);
        return true;
    }

    // Session end: presence/application set by a server's scripts must not outlive it. Only resets when
    // scripts actually changed something, since reset_application() rebuilds the whole client.
    bool Discord::reset_if_customized() {
        if (!script_controlled && application_id == default_application_id) return false;
        return reset_application();
    }
}
#endif