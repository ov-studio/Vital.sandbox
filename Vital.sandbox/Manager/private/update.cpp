/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: update.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 02/11/2026
     Desc: Update Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#include <Vital.sandbox/Manager/public/update.h>


////////////////////////////
// Vital: Manager: Update //
////////////////////////////

// TODO: Improve
namespace Vital::Manager {
    // Helpers //
    void Update::log(const std::string& mode, const std::string& message) {
        Tool::print(mode, fmt::format("{}: {}", Name, message));
    }

    // Release assets are named per operating system: Vital.sandbox-<tag>-<system>-<client|server>.zip
    const char* Update::get_system() {
        #if defined(_WIN32)
        return "windows";
        #elif defined(__APPLE__)
        return "macos";
        #else
        return "linux";
        #endif
    }

    // Names that come from the network end up in file paths: letters, digits, '.', '_' and '-' only
    bool Update::is_safe_name(const std::string& value) {
        if (value.empty() || value.size() > 128 || value.front() == '.') return false;
        for (const char c : value) {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-')) return false;
        }
        return true;
    }

    std::string Update::to_native(std::string path) {
        #if defined(_WIN32)
        std::replace(path.begin(), path.end(), '/', '\\');
        #endif
        return path;
    }

    // Downloaded updates live next to the executable: <install>/updates
    std::filesystem::path Update::get_directory() {
        return std::filesystem::path(Tool::get_directory("updates"));
    }

    void Update::write_pending(const std::string& tag, const std::string& file) {
        rapidjson::Document doc;
        doc.SetObject();
        auto& a = doc.GetAllocator();
        doc.AddMember("name", "Vital.sandbox", a);
        doc.AddMember("tag", rapidjson::Value(tag.c_str(), a), a);
        doc.AddMember("file", rapidjson::Value(file.c_str(), a), a);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        std::ofstream out(get_directory() / "pending.json", std::ios::binary | std::ios::trunc);
        if (out) out << buffer.GetString();
    }

    // Hands over to a detached script: it waits for this process to exit, copies the extracted update over
    // the install folder (the running exe/dlls are locked until then) and starts the game or server again.
    bool Update::launch(const std::filesystem::path& src, const std::filesystem::path& zip, const std::filesystem::path& pending) {
        auto* os = godot::OS::get_singleton();
        const std::string exe = to_native(Tool::to_std_string(os -> get_executable_path()));
        const std::string dst = to_native(Tool::to_std_string(os -> get_executable_path().get_base_dir()));
        const int64_t pid = os -> get_process_id();
        const auto dir = get_directory();
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

    // Same rule Kit::ensure() uses: any difference from the latest release tag means "not the latest"
    bool Update::is_outdated(const std::string& local, const std::string& remote) {
        return !local.empty() && !remote.empty() && local != remote;
    }

    // Untagged local builds (CI never injected a version) never update
    bool Update::is_dev_build() {
        const auto& sdk = Tool::Version::SDK;
        return sdk.major == 0 && sdk.minor == 0 && sdk.patch == 0 && sdk.label == "dev";
    }


    // Managers //
    Update::Release Update::fetch_release() {
        Release out;
        try {
            rapidjson::Document doc;
            doc.Parse(Tool::HTTP::get(sandbox_src).c_str());
            if (doc.HasParseError() || !doc.IsObject()) return out;
            if (doc.HasMember("tag_name") && doc["tag_name"].IsString()) out.tag = doc["tag_name"].GetString();
            const std::string suffix = fmt::format("-{}-{}.zip", get_system(), Tool::get_platform());
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

    // Remembers a finished download across restarts, and cleans up once that version is actually running
    Update::Pending Update::read_pending() {
        std::ifstream in(get_directory() / "pending.json", std::ios::binary);
        if (!in) return {};
        std::string raw{ std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
        rapidjson::Document doc;
        doc.Parse(raw.c_str());
        if (doc.HasParseError() || !doc.IsObject()) return {};
        if (!doc.HasMember("tag") || !doc["tag"].IsString() || !doc.HasMember("file") || !doc["file"].IsString()) return {};
        Pending out{ doc["tag"].GetString(), doc["file"].GetString() };
        std::error_code ec;
        if (out.tag == Tool::Version::SDK.to_string()) {
            in.close();
            std::filesystem::remove_all(get_directory(), ec); // update was applied: drop the leftovers
            return {};
        }
        if (!is_safe_name(out.file) || !std::filesystem::exists(get_directory() / out.file, ec)) return {};
        return out;
    }

    // Streams the release zip into <install>/updates, verified against GitHub's digest, and remembers it as pending.
    // `progress` receives the bytes received so far. Returns the final size, throws when anything goes wrong
    uint64_t Update::download(const Release& release, const std::function<void(uint64_t)>& progress) {
        if (busy.exchange(true)) throw std::runtime_error("another update is already downloading");
        struct Guard { ~Guard() { busy.store(false); } } guard;

        std::error_code ec;
        const auto dir = get_directory();
        std::filesystem::create_directories(dir, ec);
        const auto part = dir / (release.asset_name + ".part");
        const auto dest = dir / release.asset_name;
        try {
            std::ofstream out(part, std::ios::binary | std::ios::trunc);
            if (!out) throw std::runtime_error("cannot write to the updates folder");
            uint64_t received = 0;
            bool write_failed = false;
            const bool completed = Tool::HTTP::stream(release.asset_url, {}, 120, true, nullptr, Tool::HTTP::DEFAULT_MAX_DOWNLOAD, [&](const char* data, size_t len) {
                out.write(data, static_cast<std::streamsize>(len));
                if (!out) { write_failed = true; return false; }
                received += len;
                if (progress) progress(received);
                return true;
            });
            out.close();
            if (write_failed) throw std::runtime_error("disk write failed");
            if (!completed) throw std::runtime_error("download interrupted");

            // GitHub publishes a sha256 digest for release assets; verify it when present
            if (release.digest.rfind("sha256:", 0) == 0) {
                std::string expected = release.digest.substr(7);
                std::string actual = Tool::Crypto::hash_file("sha256", part.string());
                for (auto& c : expected) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                for (auto& c : actual) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                if (expected != actual) throw std::runtime_error("checksum mismatch, download discarded");
            }
            else if (release.size > 0 && received != release.size) throw std::runtime_error("incomplete download, size mismatch");

            std::filesystem::remove(dest, ec);
            ec.clear();
            std::filesystem::rename(part, dest, ec);
            if (ec) throw std::runtime_error("cannot finalise the download");
            write_pending(release.tag, release.asset_name);
            return received;
        }
        catch (...) {
            std::filesystem::remove(part, ec);
            throw;
        }
    }

    // Extracts a downloaded zip next to it (same hardened extractor Vital.kit uses). A broken zip is dropped
    bool Update::extract(const std::string& file) {
        std::error_code ec;
        const auto dir = get_directory();
        const auto zip = dir / file;
        if (std::filesystem::exists(zip, ec) && Manager::Kit::get_singleton() -> extract(zip.string(), (dir / "extracted").string())) return true;
        std::filesystem::remove(zip, ec);
        std::filesystem::remove(dir / "pending.json", ec);
        return false;
    }

    // Restarts into the updater script, which installs the extracted `file` first. An empty `file` is a plain restart,
    // which is all Vital.kit needs since Kit::ensure() re-downloads it on every launch
    bool Update::restart(const std::string& file) {
        const auto dir = get_directory();
        const auto zip = file.empty() ? std::filesystem::path() : dir / file;
        const auto src = file.empty() ? std::filesystem::path() : dir / "extracted";
        if (!launch(src, zip, dir / "pending.json")) return false;
        Engine::Core::get_singleton() -> shutdown();
        return true;
    }

    // The `update` console command: checks, downloads, extracts and restarts in one go, reporting in the console
    void Update::apply() {
        if (is_dev_build()) {
            log("warn", "this is a development build, nothing to update");
            return;
        }
        if (applying.exchange(true)) {
            log("warn", "an update is already in progress");
            return;
        }
        log("sbox", "checking for updates...");
        Tool::Thread::create([](Tool::Thread*) {
            auto fail = [](const std::string& message) {
                log("error", fmt::format("update failed ~ {}", message));
                applying.store(false);
            };
            const std::string current = Tool::Version::SDK.to_string();
            const Release release = fetch_release();
            const std::string kit_tag = std::get<0>(Manager::Kit::get_singleton() -> get_release());
            const std::string kit_local = Manager::Kit::get_singleton() -> get_version();
            if (release.tag.empty() && kit_tag.empty()) return fail("could not reach GitHub");

            const bool sandbox_old = is_outdated(current, release.tag);
            const bool kit_old = is_outdated(kit_local, kit_tag);
            if (!sandbox_old && !kit_old) {
                log("sbox", fmt::format("already up to date ~ {}", current));
                applying.store(false);
                return;
            }

            std::string file;
            if (sandbox_old) {
                if (release.asset_name.empty() || release.asset_url.empty()) return fail(fmt::format("no {} {} download in release {}", get_system(), Tool::get_platform(), release.tag));
                file = release.asset_name;

                // A finished download from an earlier session is reused
                const Pending pending = read_pending();
                if (pending.tag != release.tag || pending.file != release.asset_name) {
                    log("sbox", fmt::format("downloading {}...", release.tag));
                    try {
                        int step = 0;
                        download(release, [&](uint64_t received) {
                            const int now = release.size > 0 ? static_cast<int>(std::min<uint64_t>(received * 4 / release.size, 4)) : 0;
                            if (now <= step) return;
                            step = now;
                            log("sbox", fmt::format("downloading {} ~ {}%", release.tag, step * 25));
                        });
                    }
                    catch (const std::exception& ex) { return fail(ex.what()); }
                }
                log("sbox", "extracting update...");
                if (!extract(file)) return fail("extraction failed, run `update` again");
            }
            log("sbox", sandbox_old ? fmt::format("restarting into {}...", release.tag) : fmt::format("restarting to apply Vital.kit {}...", kit_tag));
            if (!restart(file)) return fail("could not start the updater");
        });
    }

    #if !defined(VSDK_Client)
    // Checks once now, then every few hours for as long as the server runs
    void Update::watch() {
        if (timer || is_dev_build()) return;
        check();
        timer = Tool::Timer::create([](Tool::Timer*, int) { check(); }, check_interval_ms, 0);
    }

    void Update::halt() {
        if (!timer) return;
        timer -> stop();
        timer = nullptr;
    }

    // Warns in the console when a newer Vital.sandbox or Vital.kit is out, once per version
    void Update::check() {
        if (is_dev_build() || applying.load() || checking.exchange(true)) return;
        Tool::Thread::create([](Tool::Thread*) {
            const std::string current = Tool::Version::SDK.to_string();
            const Release release = fetch_release();
            const std::string kit_tag = std::get<0>(Manager::Kit::get_singleton() -> get_release());
            const std::string kit_local = Manager::Kit::get_singleton() -> get_version();
            read_pending(); // Drops the leftovers of an update that has just been applied

            std::string sandbox_notice, kit_notice;
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (is_outdated(current, release.tag) && announced_sandbox != release.tag) {
                    announced_sandbox = release.tag;
                    sandbox_notice = fmt::format("new Vital.sandbox version available ~ {} (running {}), type `update` to install it: {}", release.tag, current, sandbox_releases);
                }
                if (is_outdated(kit_local, kit_tag) && announced_kit != kit_tag) {
                    announced_kit = kit_tag;
                    kit_notice = fmt::format("new Vital.kit version available ~ {} (running {}), it is applied on the next restart", kit_tag, kit_local);
                }
            }
            if (!sandbox_notice.empty()) log("warn", sandbox_notice);
            if (!kit_notice.empty()) log("warn", kit_notice);
            checking.store(false);
        });
    }
    #endif
}
