/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: mainmenu.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: MainMenu Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/mainmenu.h>
#include <Vital.sandbox/Engine/public/camera.h>
#include <Vital.sandbox/Manager/public/kit.h>
#include <Vital.sandbox/API/utility/input.h>
#include <Vital.sandbox/Tool/file.h>
#include <Vital.sandbox/Tool/http.h>
#include <Vital.sandbox/Tool/thread.h>
#include <Vital.sandbox/Tool/version.h>
#include <algorithm>


//////////////////////////////
// Vital: Engine: MainMenu //
//////////////////////////////

namespace Vital::Engine {
    std::atomic<float> MainMenu::draw_distance_mult { 1.0f };

    namespace {
        // TODO: Move to internal namespace like resource
        constexpr const char* settings_file = "client_settings.json";

        std::string settings_base() {
            return Tool::get_directory("data");
        }

        rapidjson::Document default_settings() {
            rapidjson::Document doc;
            doc.SetObject();
            auto& a = doc.GetAllocator();
            doc.AddMember("resolution", "1600x900", a);
            doc.AddMember("window_mode", "borderless", a);
            doc.AddMember("vsync", true, a);
            doc.AddMember("draw_distance_mult", 1.0, a);
            doc.AddMember("volume", 0.8, a);
            return doc;
        }

        rapidjson::Document load_settings() {
            auto doc = default_settings();
            try {
                const auto base = settings_base();
                if (!Tool::File::exists(base, settings_file)) return doc;
                const auto raw = Tool::File::read_text(base, settings_file);
                rapidjson::Document file;
                file.Parse(raw.c_str());
                if (file.HasParseError() || !file.IsObject()) return doc;
                for (auto it = file.MemberBegin(); it != file.MemberEnd(); ++it) {
                    rapidjson::Value key(it->name, doc.GetAllocator());
                    rapidjson::Value val(it->value, doc.GetAllocator());
                    if (doc.HasMember(key)) doc[key] = val;
                    else doc.AddMember(key, val, doc.GetAllocator());
                }
            }
            catch (...) { /* keep defaults */ }
            return doc;
        }

        void save_settings(const rapidjson::Value& settings) {
            try {
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                settings.Accept(writer);
                Tool::File::write_text(settings_base(), settings_file, buffer.GetString());
            }
            catch (...) { /* ignore write failures */ }
        }

        godot::Vector2i monitor_size() {
            auto* ds = Core::get_display_server();
            if (!ds) return godot::Vector2i(1920, 1080);
            godot::Vector2i best(0, 0);
            const int count = ds -> get_screen_count();
            for (int i = 0; i < count; i++) {
                const godot::Vector2i s = ds -> screen_get_size(i);
                if (s.x * s.y > best.x * best.y) best = s;
            }
            if (best.x <= 0 || best.y <= 0) best = ds -> screen_get_size(ds -> window_get_current_screen());
            return best;
        }

        void apply_settings(const rapidjson::Value& settings) {
            auto* ds = Core::get_display_server();
            if (!ds) return;

            // Skip re-applying window chrome when only volume/draw-distance change (slider spam).
            static std::string last_resolution;
            static std::string last_window_mode;

            std::string mode_str = last_window_mode.empty() ? "borderless" : last_window_mode;
            if (settings.HasMember("window_mode") && settings["window_mode"].IsString()) {
                mode_str = settings["window_mode"].GetString();
            }

            std::string res_str = last_resolution;
            if (settings.HasMember("resolution") && settings["resolution"].IsString()) {
                res_str = settings["resolution"].GetString();
            }

            const bool mode_changed = (mode_str != last_window_mode);
            const bool res_changed = (res_str != last_resolution);
            if (!mode_changed && !res_changed) {
                // Still apply vsync / draw / volume below.
            }
            else {
                last_window_mode = mode_str;
                last_resolution = res_str;

                const auto mon = monitor_size();
                int w = 0, h = 0;
                const auto x = res_str.find('x');
                if (x != std::string::npos) {
                    try {
                        w = std::stoi(res_str.substr(0, x));
                        h = std::stoi(res_str.substr(x + 1));
                    }
                    catch (...) { w = h = 0; }
                }
                if (w > mon.x) w = mon.x;
                if (h > mon.y) h = mon.y;
                if (w <= 0 || h <= 0) { w = mon.x; h = mon.y; }

                const int screen = ds -> window_get_current_screen();
                const godot::Vector2i screen_size = ds -> screen_get_size(screen);
                const godot::Vector2i screen_pos = ds -> screen_get_position(screen);
                const bool native = (w >= screen_size.x - 2 && h >= screen_size.y - 2);

                if (mode_str == "fullscreen") {
                    ds -> window_set_mode(godot::DisplayServer::WINDOW_MODE_EXCLUSIVE_FULLSCREEN);
                }
                else if (native) {
                    // Borderless at monitor size: use non-exclusive fullscreen so the
                    // client fills the display cleanly (no HUD clip / taskbar fight).
                    ds -> window_set_flag(godot::DisplayServer::WINDOW_FLAG_BORDERLESS, true);
                    ds -> window_set_mode(godot::DisplayServer::WINDOW_MODE_FULLSCREEN);
                }
                else {
                    // Smaller bordered-less window: size + center on the current monitor.
                    const godot::Vector2i size(w, h);
                    const godot::Vector2i current = ds -> window_get_size();
                    const godot::Vector2i pos(
                        screen_pos.x + (screen_size.x - size.x) / 2,
                        screen_pos.y + (screen_size.y - size.y) / 2
                    );
                    const bool at_native = (
                        current.x >= screen_size.x - 2 && current.y >= screen_size.y - 2
                    );
                    const bool shrinking = size.x < current.x || size.y < current.y;

                    ds -> window_set_flag(godot::DisplayServer::WINDOW_FLAG_BORDERLESS, false);
                    ds -> window_set_mode(godot::DisplayServer::WINDOW_MODE_WINDOWED);
                    ds -> window_set_size(size);
                    ds -> window_set_position(pos);

                    auto* core = Core::get_singleton();
                    if (core && at_native && shrinking) {
                        core -> enqueue([ds, size, pos]() {
                            ds -> window_set_size(size);
                            ds -> window_set_position(pos);
                            ds -> window_set_flag(godot::DisplayServer::WINDOW_FLAG_BORDERLESS, true);
                        });
                    }
                    else {
                        ds -> window_set_flag(godot::DisplayServer::WINDOW_FLAG_BORDERLESS, true);
                    }
                }
            }

            if (settings.HasMember("vsync") && settings["vsync"].IsBool()) {
                ds -> window_set_vsync_mode(
                    settings["vsync"].GetBool()
                        ? godot::DisplayServer::VSYNC_ENABLED
                        : godot::DisplayServer::VSYNC_DISABLED
                );
            }

            if (settings.HasMember("draw_distance_mult") && settings["draw_distance_mult"].IsNumber()) {
                const float mult = std::clamp(static_cast<float>(settings["draw_distance_mult"].GetDouble()), 0.1f, 2.0f);
                MainMenu::set_draw_distance_mult(mult);
            }

            if (settings.HasMember("volume") && settings["volume"].IsNumber()) {
                if (auto* as = Core::get_audio_server()) {
                    double linear = std::clamp(settings["volume"].GetDouble(), 0.0, 1.0);
                    int idx = as -> get_bus_index(godot::StringName("Master"));
                    if (idx < 0) idx = 0;
                    const float db = (linear <= 0.0001)
                        ? -80.0f
                        : godot::Math::linear2db(static_cast<float>(linear));
                    as -> set_bus_volume_db(idx, db);
                }
            }
        }

        // Update check: compares the running Vital.sandbox / Vital.kit against their latest GitHub release.
        constexpr const char* sandbox_src = "https://api.github.com/repos/ov-studio/Vital.sandbox/releases/latest";
        constexpr const char* sandbox_releases = "https://github.com/ov-studio/Vital.sandbox/releases";
        constexpr const char* kit_releases = "https://github.com/ov-studio/Vital.kit/releases";
        std::mutex update_mutex;
        std::string update_json;
        std::atomic<bool> update_started { false };

        std::string fetch_sandbox_tag() {
            try {
                rapidjson::Document doc;
                doc.Parse(Tool::HTTP::get(sandbox_src).c_str());
                if (!doc.HasParseError() && doc.IsObject() && doc.HasMember("tag_name") && doc["tag_name"].IsString()) return doc["tag_name"].GetString();
            }
            catch (...) { /* offline / rate limited: no update info */ }
            return "";
        }

        bool is_outdated(const std::string& local, const std::string& remote) {
            if (local.empty() || remote.empty()) return false;
            Tool::Version::Info a{}, b{};
            if (Tool::Version::Info::parse(local, a) && Tool::Version::Info::parse(remote, b)) return a < b;
            return local != remote;
        }

        // Only outdated components are listed; an empty list means "up to date" (or unknown) and the UI hides the button.
        std::string update_to_json(const std::vector<std::array<std::string, 4>>& updates) {
            rapidjson::Document envelope;
            envelope.SetObject();
            auto& a = envelope.GetAllocator();
            envelope.AddMember("action", "update", a);
            rapidjson::Value list(rapidjson::kArrayType);
            for (const auto& u : updates) {
                rapidjson::Value item(rapidjson::kObjectType);
                item.AddMember("name", rapidjson::Value(u[0].c_str(), a), a);
                item.AddMember("current", rapidjson::Value(u[1].c_str(), a), a);
                item.AddMember("latest", rapidjson::Value(u[2].c_str(), a), a);
                item.AddMember("url", rapidjson::Value(u[3].c_str(), a), a);
                list.PushBack(item, a);
            }
            envelope.AddMember("updates", list, a);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            envelope.Accept(writer);
            return buffer.GetString();
        }

        std::string settings_to_json(const rapidjson::Value& settings) {
            rapidjson::Document envelope;
            envelope.SetObject();
            auto& a = envelope.GetAllocator();
            envelope.AddMember("action", "settings", a);
            rapidjson::Value copy(settings, a);
            const auto mon = monitor_size();
            copy.AddMember("max_width", mon.x, a);
            copy.AddMember("max_height", mon.y, a);
            envelope.AddMember("settings", copy, a);
            envelope.AddMember("username", rapidjson::Value(Tool::get_username().c_str(), a), a);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            envelope.Accept(writer);
            return buffer.GetString();
        }
    }


    // Instantiators //
    MainMenu::MainMenu() {
        Engine::Webview::Options options;
        options.z_index = Engine::Webview::system_z_floor + 1;
        options.fullscreen = true;
        options.transparent = true;
        options.incognito = true;
        options.autoplay = false;
        options.zoomable = false;
        options.forward_input = false;
        options.overlay = false;
        webview = Engine::Webview::create(options);
        webview -> set_position({0, 0});
        webview -> set_handler("message", [this](Engine::Webview::Payload payload) {
            if (auto content = std::get_if<std::string>(&payload)) on_message(Tool::to_godot_string(*content));
        });

        Tool::Event::bind("kit:ready", [this](Tool::Stack arguments) {
            Engine::Core::get_singleton() -> enqueue([this]() {
                apply_settings(load_settings());
                //webview -> load_url(Engine::Core::get_singleton() -> get_http_url("cache/Vital.kit/mainmenu/build/index.html"));
                webview -> load_url("http://localhost:5173/"); // TODO: SWAP FOR ABOVE IN PRODUCTION
            });
        });
    }

    MainMenu::~MainMenu() {
        if (webview) {
            webview -> destroy();
            webview = nullptr;
        }
    }


    // Managers //
    bool MainMenu::is_ready() {
        return webview_ready.load();
    }

    bool MainMenu::is_visible() {
        return webview -> is_visible();
    }

    void MainMenu::ready() {
        webview_ready.store(true);
        if (webview) webview -> emit(settings_to_json(load_settings()));
        {
            std::lock_guard<std::mutex> lock(update_mutex);
            if (!update_json.empty()) {
                if (webview) webview -> emit(update_json);
                return;
            }
        }
        if (update_started.exchange(true)) return;
        Tool::Thread::create([this](Tool::Thread*) {
            std::vector<std::array<std::string, 4>> updates;
            bool reached = false;

            // Vital.sandbox: CI-injected SDK version vs latest release tag. Untagged dev builds (0.0.0) are skipped.
            const auto& sdk = Tool::Version::SDK;
            const std::string sandbox_tag = fetch_sandbox_tag();
            if (!sandbox_tag.empty()) reached = true;
            if (!(sdk.major == 0 && sdk.minor == 0 && sdk.patch == 0) && is_outdated(sdk.to_string(), sandbox_tag)) {
                updates.push_back({ "Vital.sandbox", sdk.to_string(), sandbox_tag, sandbox_releases });
            }

            // Vital.kit: cached kit version vs latest release tag (same source Kit::ensure uses).
            const std::string kit_tag = std::get<0>(Manager::Kit::fetch_release());
            if (!kit_tag.empty()) reached = true;
            const std::string kit_local = Manager::Kit::get_version();
            if (is_outdated(kit_local, kit_tag)) {
                updates.push_back({ "Vital.kit", kit_local, kit_tag, kit_releases });
            }

            if (!reached) { update_started.store(false); return; } // offline: retry on the next ready()
            const std::string json = update_to_json(updates);
            {
                std::lock_guard<std::mutex> lock(update_mutex);
                update_json = json;
            }
            if (webview) webview -> emit(json);
        });
    }

    void MainMenu::show() {
        if (is_visible()) return;
        webview -> set_visible(true);
        Sandbox::API::Input::push_sandbox_ui_visible();
    }

    void MainMenu::hide() {
        if (!is_visible()) return;
        webview -> set_visible(false);
        Sandbox::API::Input::pop_sandbox_ui_visible();
    }

    float MainMenu::get_draw_distance_mult() {
        return draw_distance_mult.load();
    }

    void MainMenu::set_draw_distance_mult(float mult) {
        mult = std::clamp(mult, 0.1f, 2.0f);
        const float prev = draw_distance_mult.exchange(mult);
        if (auto* cam = Camera::get_active()) {
            const float logical = (prev > 0.0001f) ? (cam -> get_far() / prev) : cam -> get_far();
            cam -> set_far(logical * mult);
        }
    }


    // Events //
    void MainMenu::on_message(godot::String message) {
        rapidjson::Document document;
        document.Parse(Tool::to_std_string(message).c_str());
        if (document.HasParseError() || !document.HasMember("action")) return;
        std::string action = document["action"].GetString();
        if (action == "ready") ready();
        else if (action == "hide") hide();
        else if (action == "drag") {
            auto* ds = Core::get_display_server();
            const auto mode = ds -> window_get_mode();
            // No move-drag in exclusive or borderless-native fullscreen.
            if (mode == godot::DisplayServer::WINDOW_MODE_EXCLUSIVE_FULLSCREEN
                || mode == godot::DisplayServer::WINDOW_MODE_FULLSCREEN) {
                return;
            }
            ds -> window_start_drag();
        }
        else if (action == "exit") Core::get_singleton() -> shutdown();
        else if (action == "open_url") {
            // Opens in the OS default browser. Web links only: never hand arbitrary schemes/paths to the shell.
            if (!document.HasMember("url") || !document["url"].IsString()) return;
            const std::string url = document["url"].GetString();
            if (url.rfind("https://", 0) != 0 && url.rfind("http://", 0) != 0) return;
            godot::OS::get_singleton() -> shell_open(Tool::to_godot_string(url));
        }
        else if (action == "settings_update") {
            if (!document.HasMember("settings") || !document["settings"].IsObject()) return;
            auto merged = default_settings();
            const auto& incoming = document["settings"];
            for (auto it = incoming.MemberBegin(); it != incoming.MemberEnd(); ++it) {
                rapidjson::Value key(it->name, merged.GetAllocator());
                rapidjson::Value val(it->value, merged.GetAllocator());
                if (merged.HasMember(key)) merged[key] = val;
                else merged.AddMember(key, val, merged.GetAllocator());
            }
            apply_settings(merged);
            save_settings(merged);
        }
    }
}
#endif
