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
    namespace {
        std::string or_placeholder(const std::string& text) {
            return text.empty() ? std::string(Discord::NO_TEXT) : text;
        }
    }


    // Instantiators //
    Discord::~Discord() {
        client.reset();
    }


    // Helpers //
    Discord::Activity Discord::get_base_activity() const {
        return server.joined ? server.activity : defaults.activity;
    }

    void Discord::build_server_activity() {
        server.activity = defaults.activity;
        server.activity.details = server.name.empty() ? "Playing on a server" : server.name;
        if (server.players >= 0 && server.max_peers > 0) server.activity.state = fmt::format("Players: {}/{}", server.players, server.max_peers);
        else if (server.players >= 0) server.activity.state = fmt::format("Players: {}", server.players);
        else server.activity.state = "In Game";
        server.activity.timestamp_start = server.started_at;
        if (server.logo.rfind("https://", 0) == 0 && server.logo.size() <= 256) {
            server.activity.large_image_key = server.logo;
            server.activity.large_image_text = or_placeholder(server.name);
            if (!defaults.brand_logo.empty()) {
                server.activity.small_image_key = defaults.brand_logo;
                server.activity.small_image_text = or_placeholder(defaults.brand_name);
            }
        }
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

    void Discord::show(const Activity& target) {
        activity = target;
        update();
    }

    template <typename F>
    bool Discord::customize(F&& fn) {
        script_controlled = true;
        fn(activity);
        update();
        return true;
    }


    // Managers //
    void Discord::init() {
        auto kit = Manager::Kit::get_singleton();
        auto read = [&](const char* key) {
            try { return kit -> get_json_value("config/discord", key).as<std::string>(); }
            catch (...) { return std::string(); }
        };
        defaults.application_id = static_cast<uint64_t>(std::stoull(read("application_id")));
        defaults.activity = { read("state"), read("details") };

        const std::string logo = read("logo");
        defaults.brand_name = read("name");
        if (!logo.empty()) {
            defaults.brand_logo = kit -> get_url(logo);
            defaults.activity.large_image_key = defaults.brand_logo;
            defaults.activity.large_image_text = or_placeholder(defaults.brand_name);
        }
        reset_application();
    }

    void Discord::process() {
        discordpp::RunCallbacks();
        if (!server.joined) return;

        if (server.info_request && server.info_request -> done.load()) {
            auto request = std::move(server.info_request);
            server.info_request.reset();
            rapidjson::Document doc;
            if (!script_controlled && !doc.Parse(request -> body.c_str()).HasParseError() && doc.IsObject()) {
                bool changed = false;
                auto sync = [&](const char* key, auto& field) {
                    if (!doc.HasMember(key)) return;
                    const auto& value = doc[key];
                    if constexpr (std::is_same_v<std::decay_t<decltype(field)>, std::string>) {
                        if (!value.IsString() || field == value.GetString()) return;
                        field = value.GetString();
                    }
                    else {
                        if (!value.IsInt() || field == value.GetInt()) return;
                        field = value.GetInt();
                    }
                    changed = true;
                };
                sync("name", server.name);
                sync("logo", server.logo);
                sync("players", server.players);
                sync("max_peers", server.max_peers);
                if (changed) {
                    build_server_activity();
                    show(server.activity);
                }
            }
        }

        if (script_controlled || server.info_request || server.info_url.empty()) return;
        auto now = static_cast<uint64_t>(godot::Time::get_singleton() -> get_ticks_msec());
        if (now < server.next_refresh) return;
        server.next_refresh = now + SERVER_REFRESH_MS;
        auto request = std::make_shared<InfoRequest>();
        server.info_request = request;
        std::thread([request, url = server.info_url]() {
            try { request -> body = Tool::HTTP::get(url, {}, 3); }
            catch (...) { request -> body.clear(); }
            request -> done = true;
        }).detach();
    }

    void Discord::enter_server() {
        auto nm = Manager::Network::get_singleton();
        server = {};
        server.joined = true;
        script_controlled = false;
        server.name = nm -> get_server_name();
        server.logo = nm -> get_server_logo_url();
        server.players = nm -> get_server_players();
        server.max_peers = nm -> get_server_max_peers();
        server.started_at = static_cast<int64_t>(godot::Time::get_singleton() -> get_unix_time_from_system());
        server.info_url = nm -> get_server_http_port() > 0 ? fmt::format("http://{}:{}/info", nm -> get_server_ip(), nm -> get_server_http_port()) : "";
        server.next_refresh = static_cast<uint64_t>(godot::Time::get_singleton() -> get_ticks_msec()) + SERVER_REFRESH_MS;
        build_server_activity();
        show(server.activity);
    }

    void Discord::leave_server() {
        server = {};
        if (reset_if_customized()) return;
        show(defaults.activity);
    }

    void Discord::update() {
        if (!client) return;
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
        if (!client) return false;
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

    bool Discord::set_state(const std::string& state) {
        return customize([&](Activity& a) { 
            a.state = state; 
        });
    }

    bool Discord::set_details(const std::string& details) {
        return customize([&](Activity& a) { 
            a.details = details; 
        });
    }

    bool Discord::set_small_image(const std::string& key, const std::string& text) {
        return customize([&](Activity& a) { 
            a.small_image_key = key; 
            a.small_image_text = text; 
        });
    }

    bool Discord::set_large_image(const std::string& key, const std::string& text) {
        return customize([&](Activity& a) { 
            a.large_image_key = key; 
            a.large_image_text = text; 
        });
    }

    bool Discord::set_timestamps(int64_t start_at, int64_t end_at) {
        auto now = static_cast<int64_t>(godot::Time::get_singleton() -> get_unix_time_from_system());
        return customize([&](Activity& a) {
            a.timestamp_start = now + start_at;
            a.timestamp_end = end_at > 0 ? now + end_at : 0;
        });
    }

    bool Discord::reset_application() {
        script_controlled = false;
        apply_application(defaults.application_id, false, false);
        return true;
    }

    bool Discord::reset_if_customized() {
        if (!script_controlled) return false;
        if (application_id != defaults.application_id) return reset_application();
        script_controlled = false;
        show(get_base_activity());
        return true;
    }
}
#endif
