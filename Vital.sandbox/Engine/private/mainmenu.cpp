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
#include <Vital.sandbox/Manager/public/kit.h>
#include <Vital.sandbox/API/utility/input.h>
#include <Vital.sandbox/Tool/file.h>


//////////////////////////////
// Vital: Engine: MainMenu //
//////////////////////////////

namespace Vital::Engine {
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
            doc.AddMember("quality", "medium", a);
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
            // Use the largest connected screen so multi-monitor setups still list valid sizes.
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

            if (settings.HasMember("window_mode") && settings["window_mode"].IsString()) {
                const std::string mode = settings["window_mode"].GetString();
                if (mode == "fullscreen") {
                    ds -> window_set_mode(godot::DisplayServer::WINDOW_MODE_EXCLUSIVE_FULLSCREEN);
                }
                else {
                    // Always borderless — custom HUD title bar owns window chrome.
                    ds -> window_set_mode(godot::DisplayServer::WINDOW_MODE_WINDOWED);
                    ds -> window_set_flag(godot::DisplayServer::WINDOW_FLAG_BORDERLESS, true);
                }
            }

            if (settings.HasMember("resolution") && settings["resolution"].IsString()) {
                const std::string res = settings["resolution"].GetString();
                const auto x = res.find('x');
                if (x != std::string::npos) {
                    try {
                        int w = std::stoi(res.substr(0, x));
                        int h = std::stoi(res.substr(x + 1));
                        const auto mon = monitor_size();
                        if (w > mon.x) w = mon.x;
                        if (h > mon.y) h = mon.y;
                        if (w > 0 && h > 0) {
                            const auto mode = ds -> window_get_mode();
                            if (mode != godot::DisplayServer::WINDOW_MODE_EXCLUSIVE_FULLSCREEN) {
                                const godot::Vector2i size(w, h);
                                const int screen = ds -> window_get_current_screen();
                                const godot::Vector2i screen_size = ds -> screen_get_size(screen);
                                const godot::Vector2i current = ds -> window_get_size();
                                const godot::Vector2i pos(
                                    (screen_size.x - size.x) / 2,
                                    (screen_size.y - size.y) / 2
                                );

                                // Borderless at native monitor size is treated as fullscreen by
                                // Windows; same-frame size changes are ignored. Drop borderless,
                                // force WINDOWED, set size, then restore borderless next tick.
                                const bool at_native = (
                                    current.x >= screen_size.x - 2 && current.y >= screen_size.y - 2
                                ) || (
                                    current.x >= mon.x - 2 && current.y >= mon.y - 2
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
                    }
                    catch (...) { /* bad resolution string */ }
                }
            }

            if (settings.HasMember("vsync") && settings["vsync"].IsBool()) {
                ds -> window_set_vsync_mode(
                    settings["vsync"].GetBool()
                        ? godot::DisplayServer::VSYNC_ENABLED
                        : godot::DisplayServer::VSYNC_DISABLED
                );
            }
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
        // Push current settings into the UI so the settings panel matches disk.
        if (webview) webview -> emit(settings_to_json(load_settings()));
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


    // Events //
    void MainMenu::on_message(godot::String message) {
        rapidjson::Document document;
        document.Parse(Tool::to_std_string(message).c_str());
        if (document.HasParseError() || !document.HasMember("action")) return;
        std::string action = document["action"].GetString();
        if (action == "ready") ready();
        else if (action == "hide") hide();
        else if (action == "drag") Core::get_display_server() -> window_start_drag();
        else if (action == "exit") Core::get_singleton() -> shutdown();
        else if (action == "settings_update") {
            if (!document.HasMember("settings") || !document["settings"].IsObject()) return;
            // Merge onto defaults so partial payloads stay valid.
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
