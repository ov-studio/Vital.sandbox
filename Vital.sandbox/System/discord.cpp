/*----------------------------------------------------------------
    Resource: Vital.sandbox
    Script: System: private: discord.cpp
    Author: ov-studio
    Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene, Ianec
    DOC: 15/01/2026
    Desc: Discord Rich Presence System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if defined(Vital_SDK_Client)
#pragma once
#define DISCORDPP_IMPLEMENTATION
#include <Vital.sandbox/System/discord.h>
#include <Vital.extension/Engine/public/console.h>


/////////////////////////////
// Vital: System: Discord //
/////////////////////////////

namespace Vital::System {
    // Instantiators //
    Discord::Discord() {
        default_application_id = static_cast<uint64_t>(std::stoull(Vital::Tool::fetch_config("discord", "application_id").as<std::string>()));
        default_activity = {
            Vital::Tool::fetch_config("discord", "state").as<std::string>(),
            Vital::Tool::fetch_config("discord", "details").as<std::string>()
        };
        reset_application();
    }

    Discord::~Discord() {
        client.reset();
    }


    // Utils //
    Discord* Discord::get_singleton() {
        if (!singleton) singleton = new Discord();
        return singleton;
    }

    void Discord::free_singleton() {
        if (!singleton) return;
        delete singleton;
        singleton = nullptr;
    }


    // Managers //
    void Discord::process() {
        discordpp::RunCallbacks();
    }

    void Discord::update() {
        discordpp::Activity client_activity;
        discordpp::ActivityAssets client_assets;
        discordpp::ActivityTimestamps client_timestamps;
        client_activity.SetType(discordpp::ActivityTypes::Playing);
        client_activity.SetState(activity.state);
        client_activity.SetDetails(activity.details);
        if (!activity.largeimage_key.empty()) {
            client_assets.SetLargeImage(activity.largeimage_key);
            client_assets.SetLargeText(activity.largeimage_text);
        }
        if (!activity.smallimage_key.empty()) {
            client_assets.SetSmallImage(activity.smallimage_key);
            client_assets.SetSmallText(activity.smallimage_text);
        }
        client_activity.SetAssets(client_assets);
        if (activity.timestamp_start > 0) client_timestamps.SetStart(activity.timestamp_start);
        if (activity.timestamp_end > 0) client_timestamps.SetEnd(activity.timestamp_end);
        client_activity.SetTimestamps(client_timestamps);
        client -> UpdateRichPresence(client_activity, [](const discordpp::ClientResult& result) {
            if (!result.Successful()) {
                Vital::print("error", "[Discord] RPC update failed ~", result.ToString());
            }
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
                    Vital::print("warn", "[Discord] Authorization rejected ~ Retrying");
                    authorize(token_directory, token_file, force_reauth);
                }
                else {
                    Vital::print("warn", "[Discord] Authorization rejected ~ Presence only");
                    client -> Connect();
                }
                return;
            }
            client -> GetToken(application_id, code, verifier.Verifier(), redirectUri, [this, token_directory, token_file](discordpp::ClientResult result, std::string accessToken, std::string refreshToken, discordpp::AuthorizationTokenType tokenType, int32_t expiresIn, std::string scopes) {
                    if (!result.Successful()) {
                        Vital::print("error", "[Discord] Token exchange failed ~", result.ToString());
                        return;
                    }
                    Vital::Tool::File::write_text(token_directory, token_file, accessToken);
                    client -> UpdateToken(tokenType, accessToken, [this](discordpp::ClientResult result) {
                        if (!result.Successful()) {
                            Vital::print("error", "[Discord] Token update failed ~", result.ToString());
                            return;
                        }
                        client -> Connect();
                    });
                }
            );
        });
    }


    // Checkers //
    bool Discord::is_connected() {
        auto status = client -> GetStatus();
        return status == discordpp::Client::Status::Ready || status == discordpp::Client::Status::Connected;
    }


    // Setters //
    bool Discord::set_application_id(uint64_t id, bool authenticate, bool force_reauth) {
        application_id = id;
        client.reset();
        client = std::make_shared<discordpp::Client>();
        activity = default_activity;
        client -> SetApplicationId(application_id);
        client -> SetStatusChangedCallback([this](discordpp::Client::Status status, discordpp::Client::Error, int32_t errorCode) {
            Vital::print("info", "[Discord] Status ~", std::string(discordpp::EnumToString(status)), "| Code ~", errorCode);
            if (status == discordpp::Client::Status::Ready || status == discordpp::Client::Status::Connected)
                update();
        });
        if (!authenticate) {
            client -> Connect();
            return true;
        }
        auto token_directory = Vital::get_directory("data", "discord");
        auto token_file = std::to_string(application_id) + ".token";
        std::string token_value = Vital::Tool::File::exists(token_directory, token_file) ? Vital::Tool::File::read_text(token_directory, token_file) : "";
        if (!token_value.empty()) {
            client -> UpdateToken(discordpp::AuthorizationTokenType::Bearer, token_value, [this, token_directory, token_file, force_reauth](discordpp::ClientResult result) {
                if (!result.Successful()) {
                    Vital::print("warn", "[Discord] Cached token expired ~ Re-authenticating");
                    Vital::Tool::File::remove(token_directory, token_file);
                    authorize(token_directory, token_file, force_reauth);
                    return;
                }
                client -> Connect();
            });
            return true;
        }
        Vital::print("info", "[Discord] Awaiting authorization");
        authorize(token_directory, token_file, force_reauth);
        return true;
    }

    bool Discord::set_activity(const Activity& data) {
        activity = data;
        update();
        return true;
    }

    bool Discord::reset_application() {
        set_application_id(default_application_id);
        return true;
    }

    bool Discord::set_state(const std::string& state) {
        activity.state = state;
        update();
        return true;
    }

    bool Discord::set_details(const std::string& details) {
        activity.details = details;
        update();
        return true;
    }

    bool Discord::set_largeimage(const std::string& key, const std::string& text) {
        activity.largeimage_key = key;
        activity.largeimage_text = text;
        update();
        return true;
    }

    bool Discord::set_smallimage(const std::string& key, const std::string& text) {
        activity.smallimage_key = key;
        activity.smallimage_text = text;
        update();
        return true;
    }

    bool Discord::set_timestamps(int64_t start_at, int64_t end_at) {
        auto now = static_cast<int64_t>(godot::Time::get_singleton() -> get_unix_time_from_system());
        activity.timestamp_start = now + start_at;
        activity.timestamp_end = end_at > 0 ? now + end_at : 0;
        update();
        return true;
    }


    // Getters //
    uint64_t Discord::get_userid() {
        if (!is_connected()) return 0;
        auto user = client -> GetCurrentUser();
        return user.Id();
    }

    std::string Discord::get_username() {
        if (!is_connected()) return "";
        auto user = client -> GetCurrentUser();
        return user.Username();
    }
}
#endif