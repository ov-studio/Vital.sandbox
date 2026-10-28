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
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/API/utility/input.h>
#include <Vital.sandbox/Tool/file.h>
#include <Vital.sandbox/Tool/http.h>
#include <Vital.sandbox/Tool/localserver.h>
#include <Vital.sandbox/Tool/thread.h>
#include <Vital.sandbox/Tool/version.h>


//////////////////////////////
// Vital: Engine: MainMenu //
//////////////////////////////

namespace Vital::Engine {
    std::atomic<float> MainMenu::draw_distance { 1.0f };
    std::atomic<float> MainMenu::particle_distance { 1.0f };
    std::atomic<float> MainMenu::mouse_horizontal_senstivity { 1.0f };
    std::atomic<float> MainMenu::mouse_vertical_senstivity { 1.0f };

    namespace {
        // TODO: Move to internal namespace like resource
        constexpr const char* settings_file = "settings.json";

        std::string settings_base() {
            return Tool::get_directory("data");
        }

        rapidjson::Document default_settings() {
            rapidjson::Document doc;
            doc.SetObject();
            auto& a = doc.GetAllocator();
            doc.AddMember("nickname", "", a);
            doc.AddMember("resolution", "1600x900", a);
            doc.AddMember("window_mode", "borderless", a);
            doc.AddMember("vsync", true, a);
            doc.AddMember("draw_distance", 1.0, a);
            doc.AddMember("particle_distance", 1.0, a);
            doc.AddMember("volume", 1.0, a);
            doc.AddMember("mouse_horizontal_senstivity", 1.0, a);
            doc.AddMember("mouse_vertical_senstivity", 1.0, a);
            doc.AddMember("favorites", rapidjson::Value(rapidjson::kArrayType), a);
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

            if (settings.HasMember("draw_distance") && settings["draw_distance"].IsNumber()) {
                const float mult = std::clamp(static_cast<float>(settings["draw_distance"].GetDouble()), 0.1f, 2.0f);
                MainMenu::set_draw_distance(mult);
            }

            if (settings.HasMember("particle_distance") && settings["particle_distance"].IsNumber()) {
                MainMenu::set_particle_distance(static_cast<float>(settings["particle_distance"].GetDouble()));
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

            if (settings.HasMember("mouse_horizontal_senstivity") && settings["mouse_horizontal_senstivity"].IsNumber()) {
                MainMenu::set_mouse_horizontal_senstivity(static_cast<float>(settings["mouse_horizontal_senstivity"].GetDouble()));
            }
            if (settings.HasMember("mouse_vertical_senstivity") && settings["mouse_vertical_senstivity"].IsNumber()) {
                MainMenu::set_mouse_vertical_senstivity(static_cast<float>(settings["mouse_vertical_senstivity"].GetDouble()));
            }
        }

        // Update check: compares the running Vital.sandbox / Vital.kit against their latest GitHub release.
        constexpr const char* sandbox_src = "https://api.github.com/repos/ov-studio/Vital.sandbox/releases/latest";
        constexpr const char* sandbox_releases = "https://github.com/ov-studio/Vital.sandbox/releases";
        constexpr const char* kit_releases = "https://github.com/ov-studio/Vital.kit/releases";
        constexpr int64_t update_cooldown_ms = 10 * 60 * 1000; // min delay between checks (GitHub API is rate limited per IP)
        constexpr int64_t update_retry_ms = 60 * 1000;         // shorter delay after a failed (offline) check

        // One outdated component. state: idle | downloading | ready | installing | error
        struct UpdateEntry {
            std::string name, current, latest, url;
            std::string state = "idle";
            std::string message;
            uint64_t received = 0;
            uint64_t total = 0;
            std::string asset_name, asset_url, digest;         // Vital.sandbox only (release zip for this platform)
        };

        std::mutex update_mutex;
        std::string update_json;                               // last result, re-sent when the UI reloads
        std::vector<UpdateEntry> update_entries;               // guarded by update_mutex
        std::atomic<bool> update_running { false };
        std::atomic<int64_t> update_next { 0 };                // earliest time (steady clock, ms) of the next check
        bool staged_loaded = false;
        std::string staged_tag, staged_file;                   // update already downloaded into updates/ by an earlier session

        int64_t now_ms() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        const char* platform_name() {
            #if defined(_WIN32)
            return "windows";
            #elif defined(__APPLE__)
            return "macos";
            #else
            return "linux";
            #endif
        }

        // Names that come from the network end up in file paths: letters, digits, '.', '_' and '-' only.
        bool is_safe_name(const std::string& value) {
            if (value.empty() || value.size() > 128 || value.front() == '.') return false;
            for (const char c : value) {
                if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-')) return false;
            }
            return true;
        }

        struct SandboxRelease {
            std::string tag, asset_name, asset_url, digest;
            uint64_t size = 0;
        };

        SandboxRelease fetch_sandbox_release() {
            SandboxRelease out;
            try {
                rapidjson::Document doc;
                doc.Parse(Tool::HTTP::get(sandbox_src).c_str());
                if (doc.HasParseError() || !doc.IsObject()) return out;
                if (doc.HasMember("tag_name") && doc["tag_name"].IsString()) out.tag = doc["tag_name"].GetString();
                const std::string suffix = fmt::format("-{}-client.zip", platform_name());
                if (doc.HasMember("assets") && doc["assets"].IsArray()) {
                    for (const auto& asset : doc["assets"].GetArray()) {
                        if (!asset.IsObject() || !asset.HasMember("name") || !asset["name"].IsString()) continue;
                        if (!asset.HasMember("browser_download_url") || !asset["browser_download_url"].IsString()) continue;
                        const std::string name = asset["name"].GetString();
                        if (name.rfind("Vital.sandbox-", 0) != 0 || name.size() <= suffix.size()) continue;
                        if (name.compare(name.size() - suffix.size(), suffix.size(), suffix) != 0) continue;
                        if (!is_safe_name(name)) continue;
                        out.asset_name = name;
                        out.asset_url = asset["browser_download_url"].GetString();
                        if (asset.HasMember("size") && asset["size"].IsUint64()) out.size = asset["size"].GetUint64();
                        if (asset.HasMember("digest") && asset["digest"].IsString()) out.digest = asset["digest"].GetString();
                        break;
                    }
                }
            }
            catch (...) { /* offline / rate limited: no update info */ }
            return out;
        }

        // Same rule Kit::ensure() uses: any difference from the latest release tag means "not the latest".
        bool is_outdated(const std::string& local, const std::string& remote) {
            return !local.empty() && !remote.empty() && local != remote;
        }

        // Untagged local builds (CI never injected a version) never show updates.
        bool is_dev_build() {
            const auto& sdk = Tool::Version::SDK;
            return sdk.major == 0 && sdk.minor == 0 && sdk.patch == 0 && sdk.label == "dev";
        }

        // Downloaded updates live next to the executable: <install>/updates
        std::filesystem::path updates_dir() {
            return std::filesystem::path(Tool::get_directory("updates"));
        }

        void write_pending(const std::string& tag, const std::string& file) {
            rapidjson::Document doc;
            doc.SetObject();
            auto& a = doc.GetAllocator();
            doc.AddMember("name", "Vital.sandbox", a);
            doc.AddMember("tag", rapidjson::Value(tag.c_str(), a), a);
            doc.AddMember("file", rapidjson::Value(file.c_str(), a), a);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            std::ofstream out(updates_dir() / "pending.json", std::ios::binary | std::ios::trunc);
            if (out) out << buffer.GetString();
        }

        // Remembers a finished download across restarts, and cleans up once that version is actually running.
        void load_staged() {
            if (staged_loaded) return;
            staged_loaded = true;
            std::ifstream in(updates_dir() / "pending.json", std::ios::binary);
            if (!in) return;
            std::string raw{ std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
            rapidjson::Document doc;
            doc.Parse(raw.c_str());
            if (doc.HasParseError() || !doc.IsObject()) return;
            if (!doc.HasMember("tag") || !doc["tag"].IsString() || !doc.HasMember("file") || !doc["file"].IsString()) return;
            const std::string tag = doc["tag"].GetString();
            const std::string file = doc["file"].GetString();
            std::error_code ec;
            if (tag == Tool::Version::SDK.to_string()) {
                std::filesystem::remove_all(updates_dir(), ec); // update was applied: drop the leftovers
                return;
            }
            if (!is_safe_name(file) || !std::filesystem::exists(updates_dir() / file, ec)) return;
            staged_tag = tag;
            staged_file = file;
        }

        UpdateEntry* find_entry(const std::string& name) {
            for (auto& e : update_entries) if (e.name == name) return &e;
            return nullptr;
        }

        std::string update_to_json(const std::vector<UpdateEntry>& updates) {
            rapidjson::Document envelope;
            envelope.SetObject();
            auto& a = envelope.GetAllocator();
            envelope.AddMember("action", "update", a);
            rapidjson::Value list(rapidjson::kArrayType);
            for (const auto& u : updates) {
                rapidjson::Value item(rapidjson::kObjectType);
                item.AddMember("name", rapidjson::Value(u.name.c_str(), a), a);
                item.AddMember("current", rapidjson::Value(u.current.c_str(), a), a);
                item.AddMember("latest", rapidjson::Value(u.latest.c_str(), a), a);
                item.AddMember("url", rapidjson::Value(u.url.c_str(), a), a);
                item.AddMember("state", rapidjson::Value(u.state.c_str(), a), a);
                item.AddMember("message", rapidjson::Value(u.message.c_str(), a), a);
                item.AddMember("received", static_cast<uint64_t>(u.received), a);
                item.AddMember("total", static_cast<uint64_t>(u.total), a);
                list.PushBack(item, a);
            }
            envelope.AddMember("updates", list, a);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            envelope.Accept(writer);
            return buffer.GetString();
        }

        std::string to_native(std::string path) {
            #if defined(_WIN32)
            std::replace(path.begin(), path.end(), '/', '\\');
            #endif
            return path;
        }

        // Hands over to a detached script: it waits for this process to exit, copies the extracted update over
        // the install folder (the running exe/dlls are locked until then) and starts the game again.
        bool launch_updater(const std::filesystem::path& src, const std::filesystem::path& zip, const std::filesystem::path& pending) {
            auto* os = godot::OS::get_singleton();
            const std::string exe = to_native(Tool::to_std_string(os -> get_executable_path()));
            const std::string dst = to_native(Tool::to_std_string(os -> get_executable_path().get_base_dir()));
            const int64_t pid = os -> get_process_id();
            const auto dir = updates_dir();
            std::error_code ec;
            std::filesystem::create_directories(dir, ec);
            const std::string src_s = src.empty() ? "" : to_native(src.string());
            const std::string zip_s = zip.empty() ? "" : to_native(zip.string());
            const std::string pending_s = to_native(pending.string());

            #if defined(_WIN32)
            auto esc = [](std::string v) {
                std::string out;
                for (const char c : v) { out += c; if (c == '%') out += '%'; }
                return out;
            };
            const auto script = dir / "apply_update.bat";
            std::ofstream f(script, std::ios::binary | std::ios::trunc);
            if (!f) return false;
            f << "@echo off\r\n"
              << "setlocal\r\n"
              << "set \"PID=" << pid << "\"\r\n"
              << "set \"SRC=" << esc(src_s) << "\"\r\n"
              << "set \"ZIP=" << esc(zip_s) << "\"\r\n"
              << "set \"PENDING=" << esc(pending_s) << "\"\r\n"
              << "set \"DST=" << esc(dst) << "\"\r\n"
              << "set \"EXE=" << esc(exe) << "\"\r\n"
              << "set tries=0\r\n"
              << ":wait\r\n"
              << "tasklist /FI \"PID eq %PID%\" /FO CSV /NH 2>nul | find \"\"\"%PID%\"\"\" >nul\r\n"
              << "if not errorlevel 1 (\r\n"
              << "  timeout /t 1 /nobreak >nul\r\n"
              << "  goto wait\r\n"
              << ")\r\n"
              << "if not defined SRC goto launch\r\n"
              << "if not exist \"%SRC%\" goto launch\r\n"
              << ":copy\r\n"
              << "robocopy \"%SRC%\" \"%DST%\" /E /IS /IT /R:5 /W:1 /NFL /NDL /NJH /NJS /NP >nul\r\n"
              << "if %ERRORLEVEL% LSS 8 goto done\r\n"
              << "set /a tries+=1\r\n"
              << "if %tries% GEQ 5 goto launch\r\n"
              << "timeout /t 2 /nobreak >nul\r\n"
              << "goto copy\r\n"
              << ":done\r\n"
              << "rmdir /s /q \"%SRC%\" 2>nul\r\n"
              << "if defined ZIP del /q \"%ZIP%\" 2>nul\r\n"
              << "del /q \"%PENDING%\" 2>nul\r\n"
              << ":launch\r\n"
              << "start \"\" /D \"%DST%\" \"%EXE%\"\r\n"
              << "(goto) 2>nul & del \"%~f0\"\r\n";
            f.close();
            godot::PackedStringArray args;
            args.push_back("/c");
            args.push_back(Tool::to_godot_string(to_native(script.string())));
            return os -> create_process("cmd.exe", args, false) > 0;
            #else
            auto quote = [](const std::string& v) {
                std::string out = "'";
                for (const char c : v) { if (c == '\'') out += "'\\''"; else out += c; }
                return out + "'";
            };
            const auto script = dir / "apply_update.sh";
            std::ofstream f(script, std::ios::binary | std::ios::trunc);
            if (!f) return false;
            f << "#!/bin/sh\n"
              << "while kill -0 " << pid << " 2>/dev/null; do sleep 1; done\n"
              << "if [ -n " << quote(src_s) << " ] && [ -d " << quote(src_s) << " ]; then\n"
              << "  cp -Rf " << quote(src_s) << "/. " << quote(dst) << "/ && rm -rf " << quote(src_s) << " " << quote(zip_s) << " " << quote(pending_s) << "\n"
              << "fi\n"
              << "cd " << quote(dst) << " && nohup " << quote(exe) << " >/dev/null 2>&1 &\n"
              << "rm -f -- \"$0\"\n";
            f.close();
            godot::PackedStringArray args;
            args.push_back(Tool::to_godot_string(script.string()));
            return os -> create_process("/bin/sh", args, false) > 0;
            #endif
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

        for (const char* name : { "network:connect", "network:connect:success", "network:connect:failed", "network:disconnect", "network:server:disconnect" }) {
            Tool::Event::bind(name, [this](Tool::Stack) { send_connection(); });
        }

        Tool::Event::bind("network:connect:success", [this](Tool::Stack) {
            if (!is_visible() || !webview) return;
            rapidjson::Document doc;
            doc.SetObject();
            auto& a = doc.GetAllocator();
            doc.AddMember("action", "fadeout", a);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            webview -> emit(buffer.GetString());
        });

        // Leaving a server (console `disconnect`, or the server dropping us) returns to the menu, as there is no game left to show.
        for (const char* name : { "network:disconnect", "network:server:disconnect" }) {
            Tool::Event::bind(name, [this](Tool::Stack) {
                if (webview && is_ready() && !is_visible()) show();
            });
        }

        Tool::Event::bind("kit:ready", [this](Tool::Stack arguments) {
            Engine::Core::get_singleton() -> enqueue([this]() {
                apply_settings(load_settings());
                webview -> load_url(Engine::Core::get_singleton() -> get_http_url("cache/Vital.kit/ui/mainmenu/build/index.html"));
                //webview -> load_url("http://localhost:5173/"); // TODO: SWAP FOR ABOVE IN PRODUCTION
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
            if (!update_json.empty() && webview) webview -> emit(update_json);
        }
        {
            // Menu toggle key comes from Vital.kit (config/mainmenu.json), same source the engine-side key handler uses
            const auto bind = Manager::Kit::get_singleton() -> get_json_value("config/mainmenu", "bind");
            rapidjson::Document doc;
            doc.SetObject();
            auto& a = doc.GetAllocator();
            doc.AddMember("action", "init", a);
            doc.AddMember("bind", rapidjson::Value(bind.as<std::string>().c_str(), a), a);
            doc.AddMember("masterlist_refresh", Manager::Kit::get_singleton() -> get_json_value("config/masterlist", "refresh").as<int32_t>(), a);
            rapidjson::Value versions(rapidjson::kArrayType);
            for (const auto& [label, value] : Tool::Version::get_all()) {
                rapidjson::Value item(rapidjson::kObjectType);
                item.AddMember("label", rapidjson::Value(label.c_str(), a), a);
                item.AddMember("value", rapidjson::Value(value.c_str(), a), a);
                versions.PushBack(item, a);
            }
            doc.AddMember("versions", versions, a);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            if (webview) webview -> emit(buffer.GetString());
        }
        send_connection();
        check_updates();
    }

    // Pushes { action: "connection", state, ip, port }: state is "idle" | "connecting" | "connected".
    void MainMenu::send_connection() {
        if (!webview || !webview_ready.load()) return;
        auto* nm = Manager::Network::get_singleton();
        const char* state = nm -> is_connected() ? "connected" : (nm -> is_connecting() ? "connecting" : "idle");
        rapidjson::Document doc;
        doc.SetObject();
        auto& a = doc.GetAllocator();
        doc.AddMember("action", "connection", a);
        doc.AddMember("state", rapidjson::Value(state, a), a);
        doc.AddMember("ip", rapidjson::Value(nm -> get_server_ip().c_str(), a), a);
        doc.AddMember("port", nm -> get_server_port(), a);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        webview -> emit(buffer.GetString());
    }

    void MainMenu::fetch_masterlist() {
        if (!webview || masterlist_running.exchange(true)) return;
        Tool::Thread::create([this](Tool::Thread*) {
            rapidjson::Document body;
            bool ok = false;
            try {
                body.Parse(Tool::HTTP::get("https://api.vital-sandbox.com/masterlist", {}, 8).c_str());
                ok = !body.HasParseError() && body.IsArray();
            }
            catch (...) { /* offline / unavailable: the page keeps its last list and shows the error state */ }

            rapidjson::Document out;
            out.SetObject();
            auto& a = out.GetAllocator();
            out.AddMember("action", "masterlist", a);
            out.AddMember("ok", ok, a);
            if (ok) out.AddMember("servers", rapidjson::Value(body, a), a);
            else out.AddMember("servers", rapidjson::Value(rapidjson::kArrayType), a);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            out.Accept(writer);
            if (webview) webview -> emit(buffer.GetString());
            masterlist_running.store(false);
        });
    }

    // Lists servers running on this machine: every registry entry is verified against its own /info,
    // so any port works and servers that crashed (stale entries) never show up. Runs off the main thread.
    void MainMenu::scan_local_servers() {
        if (!webview || scan_running.exchange(true)) return;
        Tool::Thread::create([this](Tool::Thread*) {
            rapidjson::Document out;
            out.SetObject();
            auto& a = out.GetAllocator();
            out.AddMember("action", "localservers", a);
            rapidjson::Value servers(rapidjson::kArrayType);
            for (const auto& entry : Tool::LocalServer::list()) {
                try {
                    rapidjson::Document info;
                    info.Parse(Tool::HTTP::get(fmt::format("http://127.0.0.1:{}/info", entry.http_port), {}, 2).c_str());
                    if (info.HasParseError() || !info.IsObject()) continue;
                    // Registry entry and server must agree, else the file is stale and the port was reused by something else
                    if (!info.HasMember("port") || !info.HasMember("http_port") || !info["port"].IsInt() || !info["http_port"].IsInt()) continue;
                    if (info["port"].GetInt() != entry.port || info["http_port"].GetInt() != entry.http_port) continue;
                    rapidjson::Value item(info, a);
                    servers.PushBack(item, a);
                }
                catch (...) { /* not answering: stale or starting up */ }
            }
            out.AddMember("servers", servers, a);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            out.Accept(writer);
            if (webview) webview -> emit(buffer.GetString());
            scan_running.store(false);
        });
    }

    // Serializes the current update list, remembers it for UI reloads and pushes it to the menu.
    void MainMenu::push_updates() {
        std::string json;
        {
            std::lock_guard<std::mutex> lock(update_mutex);
            json = update_to_json(update_entries);
            update_json = json;
        }
        if (webview) webview -> emit(json);
    }

    // Throttled: runs at most once per cooldown, never concurrently, and never blocks the main thread.
    void MainMenu::check_updates() {
        if (is_dev_build()) return; // dev builds: no update info (sandbox or kit)
        const int64_t now = now_ms();
        if (now < update_next.load()) return;
        if (update_running.exchange(true)) return;
        update_next.store(now + update_cooldown_ms);
        Tool::Thread::create([this](Tool::Thread*) {
            load_staged();
            std::vector<UpdateEntry> found;
            bool reached = false;

            // Vital.sandbox: running version (to_string, e.g. v1.2.3) vs latest release tag.
            const auto& sdk = Tool::Version::SDK;
            const SandboxRelease release = fetch_sandbox_release();
            if (!release.tag.empty()) reached = true;
            if (is_outdated(sdk.to_string(), release.tag)) {
                UpdateEntry e;
                e.name = "Vital.sandbox";
                e.current = sdk.to_string();
                e.latest = release.tag;
                e.url = sandbox_releases;
                e.asset_name = release.asset_name;
                e.asset_url = release.asset_url;
                e.digest = release.digest;
                e.total = release.size;
                if (!staged_tag.empty() && staged_tag == release.tag) {
                    e.state = "ready";
                    e.message = "Ready to install";
                    e.received = e.total;
                }
                found.push_back(std::move(e));
            }

            // Vital.kit: cached kit version vs latest release tag (same source Kit::ensure uses).
            // Kit::ensure() already re-downloads and verifies it on every launch, so a restart is all it needs.
            const std::string kit_tag = std::get<0>(Manager::Kit::get_singleton() -> get_release());
            if (!kit_tag.empty()) reached = true;
            const std::string kit_local = Manager::Kit::get_singleton() -> get_version();
            if (is_outdated(kit_local, kit_tag)) {
                UpdateEntry e;
                e.name = "Vital.kit";
                e.current = kit_local;
                e.latest = kit_tag;
                e.url = kit_releases;
                found.push_back(std::move(e));
            }

            if (reached) {
                {
                    std::lock_guard<std::mutex> lock(update_mutex);
                    // A re-check must not reset a download/install that is already in flight for the same release.
                    for (auto& n : found) {
                        const auto* old = find_entry(n.name);
                        if (old && old -> latest == n.latest) {
                            n.state = old -> state;
                            n.message = old -> message;
                            n.received = old -> received;
                            n.total = old -> total;
                        }
                    }
                    update_entries = std::move(found);
                }
                push_updates();
            }
            else update_next.store(now_ms() + update_retry_ms); // offline: retry sooner
            update_running.store(false);
        });
    }

    // Downloads the release zip into <install>/updates with live progress. Nothing is installed yet.
    void MainMenu::download_update(const std::string& name) {
        UpdateEntry job;
        bool start = false;
        {
            std::lock_guard<std::mutex> lock(update_mutex);
            auto* e = find_entry(name);
            if (!e || e -> state == "downloading" || e -> state == "installing" || e -> state == "ready") return;
            if (e -> name != "Vital.sandbox") {
                e -> state = "ready";
                e -> message = "Installs automatically on restart";
            }
            else if (e -> asset_url.empty() || e -> asset_name.empty()) {
                e -> state = "error";
                e -> message = "No download available for this platform";
            }
            else {
                e -> state = "downloading";
                e -> message.clear();
                e -> received = 0;
                job = *e;
                start = true;
            }
        }
        push_updates();
        if (!start) return;

        Tool::Thread::create([this, job](Tool::Thread*) {
            auto set_state = [&](const std::string& state, const std::string& message) {
                {
                    std::lock_guard<std::mutex> lock(update_mutex);
                    auto* e = find_entry(job.name);
                    if (!e || e -> latest != job.latest) return;
                    e -> state = state;
                    e -> message = message;
                }
                push_updates();
            };

            std::error_code ec;
            const auto dir = updates_dir();
            std::filesystem::create_directories(dir, ec);
            const auto part = dir / (job.asset_name + ".part");
            const auto dest = dir / job.asset_name;
            try {
                std::ofstream out(part, std::ios::binary | std::ios::trunc);
                if (!out) throw std::runtime_error("cannot write to the updates folder");
                uint64_t received = 0;
                int64_t last_emit = 0;
                bool write_failed = false;
                const bool completed = Tool::HTTP::stream(job.asset_url, {}, 120, true, nullptr, Tool::HTTP::DEFAULT_MAX_DOWNLOAD, [&](const char* data, size_t len) {
                    out.write(data, static_cast<std::streamsize>(len));
                    if (!out) { write_failed = true; return false; }
                    received += len;
                    const int64_t t = now_ms();
                    if (t - last_emit >= 150) {
                        last_emit = t;
                        {
                            std::lock_guard<std::mutex> lock(update_mutex);
                            auto* e = find_entry(job.name);
                            if (e && e -> latest == job.latest) e -> received = received;
                        }
                        push_updates();
                    }
                    return true;
                });
                out.close();
                if (write_failed) throw std::runtime_error("disk write failed");
                if (!completed) throw std::runtime_error("download interrupted");

                // GitHub publishes a sha256 digest for release assets; verify it when present.
                if (job.digest.rfind("sha256:", 0) == 0) {
                    std::string expected = job.digest.substr(7);
                    std::string actual = Tool::Crypto::hash_file("sha256", part.string());
                    for (auto& c : expected) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                    for (auto& c : actual) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                    if (expected != actual) throw std::runtime_error("checksum mismatch, download discarded");
                }
                else if (job.total > 0 && received != job.total) throw std::runtime_error("incomplete download, size mismatch");

                std::filesystem::remove(dest, ec);
                ec.clear();
                std::filesystem::rename(part, dest, ec);
                if (ec) throw std::runtime_error("cannot finalise the download");
                write_pending(job.latest, job.asset_name);
                {
                    std::lock_guard<std::mutex> lock(update_mutex);
                    auto* e = find_entry(job.name);
                    if (e && e -> latest == job.latest) { e -> received = received; e -> total = received; }
                }
                set_state("ready", "Ready to install");
            }
            catch (const std::exception& ex) {
                std::filesystem::remove(part, ec);
                set_state("error", ex.what());
            }
        });
    }

    // Extracts the downloaded zip (same hardened extractor Vital.kit uses), then restarts into the updater script.
    void MainMenu::apply_update() {
        std::string tag, file;
        {
            std::lock_guard<std::mutex> lock(update_mutex);
            bool any = false;
            for (const auto& e : update_entries) {
                if (e.state == "installing") return;
                if (e.state != "ready") continue;
                any = true;
                if (e.name == "Vital.sandbox") { tag = e.latest; file = e.asset_name; }
            }
            if (!any) return;
            if (auto* s = find_entry("Vital.sandbox"); s && !file.empty()) {
                s -> state = "installing";
                s -> message = "Extracting update...";
            }
        }
        push_updates();

        Tool::Thread::create([this, tag, file](Tool::Thread*) {
            std::error_code ec;
            const auto dir = updates_dir();
            const auto zip = file.empty() ? std::filesystem::path() : dir / file;
            const auto src = file.empty() ? std::filesystem::path() : dir / "extracted";
            if (!file.empty()) {
                const bool ok = std::filesystem::exists(zip, ec) && Manager::Kit::get_singleton() -> extract(zip.string(), src.string());
                if (!ok) {
                    std::filesystem::remove(zip, ec);
                    std::filesystem::remove(dir / "pending.json", ec);
                    {
                        std::lock_guard<std::mutex> lock(update_mutex);
                        if (auto* s = find_entry("Vital.sandbox")) { s -> state = "error"; s -> message = "Extraction failed, download it again"; }
                    }
                    push_updates();
                    return;
                }
            }
            if (!launch_updater(src, zip, dir / "pending.json")) {
                {
                    std::lock_guard<std::mutex> lock(update_mutex);
                    if (auto* s = find_entry("Vital.sandbox")) { s -> state = "error"; s -> message = "Could not start the updater"; }
                }
                push_updates();
                return;
            }
            Engine::Core::get_singleton() -> shutdown();
        });
    }

    void MainMenu::show() {
        if (is_visible()) return;
        webview -> set_visible(true);
        webview -> set_focussed(true);
        Sandbox::API::Input::push_sandbox_ui_visible();
        check_updates();
    }

    // Esc in-game: shows the menu (it fades in on its own), or asks the UI to fade out and hide itself.
    // Only meaningful while connected: otherwise there is no game behind the menu to reveal.
    void MainMenu::toggle() {
        if (!is_ready() || !Manager::Network::get_singleton() -> is_connected()) return;
        if (!is_visible()) {
            show();
            return;
        }
        if (webview) webview -> emit("{\"action\":\"fadeout\"}");
    }

    void MainMenu::hide() {
        if (!is_visible()) return;
        webview -> set_visible(false);
        Sandbox::API::Input::pop_sandbox_ui_visible();
    }

    float MainMenu::get_draw_distance() {
        return draw_distance.load();
    }

    void MainMenu::set_draw_distance(float mult) {
        mult = std::clamp(mult, 0.1f, 2.0f);
        draw_distance.store(mult);
        if (auto cam = Engine::Camera::get_active()) cam -> apply_draw_distance();
    }

    float MainMenu::get_particle_distance() {
        return particle_distance.load();
    }

    void MainMenu::set_particle_distance(float mult) {
        particle_distance.store(std::clamp(mult, 0.1f, 2.0f));
    }

    float MainMenu::get_mouse_horizontal_senstivity() {
        return mouse_horizontal_senstivity.load();
    }

    void MainMenu::set_mouse_horizontal_senstivity(float mult) {
        mouse_horizontal_senstivity.store(std::clamp(mult, 0.01f, 2.0f));
    }

    float MainMenu::get_mouse_vertical_senstivity() {
        return mouse_vertical_senstivity.load();
    }

    void MainMenu::set_mouse_vertical_senstivity(float mult) {
        mouse_vertical_senstivity.store(std::clamp(mult, 0.01f, 2.0f));
    }


    // Events //
    void MainMenu::on_message(godot::String message) {
        rapidjson::Document document;
        document.Parse(Tool::to_std_string(message).c_str());
        if (document.HasParseError() || !document.IsObject() || !document.HasMember("action") || !document["action"].IsString()) return;
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
        else if (action == "minimize") Core::get_display_server() -> window_set_mode(godot::DisplayServer::WINDOW_MODE_MINIMIZED);
        else if (action == "exit") Core::get_singleton() -> shutdown();
        else if (action == "open_url") {
            // Opens in the OS default browser. Web links only: never hand arbitrary schemes/paths to the shell.
            if (!document.HasMember("url") || !document["url"].IsString()) return;
            const std::string url = document["url"].GetString();
            if (url.rfind("https://", 0) != 0 && url.rfind("http://", 0) != 0) return;
            godot::OS::get_singleton() -> shell_open(Tool::to_godot_string(url));
        }
        else if (action == "connect") {
            if (!document.HasMember("ip") || !document["ip"].IsString()) return;
            if (!document.HasMember("port") || !document["port"].IsInt()) return;
            const int port = document["port"].GetInt();
            const int http_port = (document.HasMember("http_port") && document["http_port"].IsInt()) ? document["http_port"].GetInt() : -1;
            Manager::Network::get_singleton() -> connect_to_server(document["ip"].GetString(), port, http_port);
        }
        else if (action == "update_download") {
            if (!document.HasMember("name") || !document["name"].IsString()) return;
            download_update(document["name"].GetString());
        }
        else if (action == "update_apply") apply_update();
        else if (action == "escape") toggle();
        else if (action == "localservers") scan_local_servers();
        else if (action == "masterlist") fetch_masterlist();
        else if (action == "disconnect") Manager::Network::get_singleton() -> disconnect_from_server();
        else if (action == "settings_update") {
            if (!document.HasMember("settings") || !document["settings"].IsObject()) return;
            auto merged = load_settings();
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
