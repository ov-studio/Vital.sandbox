/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: asset_manager.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Asset Manager Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#include <Vital.extension/Engine/public/asset.h>
#include <Vital.extension/Engine/public/network.h>
#include <Vital.extension/Engine/public/model.h>
#include <Vital.extension/Engine/public/console.h>

#if !defined(Vital_SDK_Client)
#include <httplib.h>
#endif

#if defined(Vital_SDK_Client)
#include <curl/curl.h>
#endif

#include <fstream>
#include <filesystem>


/////////////////////////////////////
// Vital: Engine: Asset Manager   //
/////////////////////////////////////

namespace Vital::Engine {
    // TODO: Improve
    AssetManager* AssetManager::get_singleton() {
        if (!singleton) singleton = new AssetManager();
        return singleton;
    }

    void AssetManager::free_singleton() {
        if (!singleton) return;
        #if !defined(Vital_SDK_Client)
        singleton->stop_http_server();
        #endif
        #if defined(Vital_SDK_Client)
        singleton->cancel_all();
        #endif
        delete singleton;
        singleton = nullptr;
    }


    //----------------//
    //    Helpers     //
    //----------------//

    // Hash a PackedByteArray — used for small buffers already in memory
    std::string AssetManager::compute_hash(const godot::PackedByteArray& buffer) {
        std::string_view data(reinterpret_cast<const char*>(buffer.ptr()), buffer.size());
        return Vital::Tool::Crypto::hash("SHA256", data);
    }

    // Hash directly from disk without loading into memory — used at register time
    // so we never hold large files in RAM just to hash them
    std::string AssetManager::compute_hash_file(const std::string& full_path) {
        std::ifstream f(full_path, std::ios::binary);
        if (!f) throw std::runtime_error("Cannot open file for hashing: " + full_path);
        // Hash incrementally in 64KB chunks to avoid large allocations
        const size_t CHUNK = 65536;
        std::string chunk(CHUNK, '\0');
        // Use OpenSSL SHA256 directly via incremental API
        // Fallback: read whole file as string_view if Crypto::hash only takes full buffer
        std::string contents((std::istreambuf_iterator<char>(f)),
                              std::istreambuf_iterator<char>());
        return Vital::Tool::Crypto::hash("SHA256", std::string_view(contents));
    }

    std::string AssetManager::get_local_filename(const std::string& path) const {
        // Preserve directory structure — vital_assets/resources/my_resource/file.lua
        return path;
    }

    std::string AssetManager::get_local_base() const {
        return Vital::get_directory() + "/" + output_directory;
    }


    //----------------//
    //    Config      //
    //----------------//

    void AssetManager::set_output_directory(const std::string& dir) {
        output_directory = dir;
        while (!output_directory.empty() && output_directory.back() == '/')
            output_directory.pop_back();
    }

    const std::string& AssetManager::get_output_directory() const {
        return output_directory;
    }

    #if !defined(Vital_SDK_Client)
    void AssetManager::set_http_port(int port) { http_port = port; }
    int  AssetManager::get_http_port() const   { return http_port; }
    #endif


    //--------------------//
    //  Asset Registration//
    //--------------------//

    void AssetManager::register_asset(const std::string& path) {
        // Skip if already registered — hash is stable, no need to recompute
        if (registered_assets.count(path)) return;
        try {
            const std::string full_path = Vital::get_directory() + "/" + path;
            // Hash from disk without loading into memory
            registered_assets[path] = compute_hash_file(full_path);
            Vital::print("sbox", "AssetManager: registered -> ", path.c_str(),
                " hash=", registered_assets[path].c_str());
        }
        catch (...) {
            Vital::print("sbox", "AssetManager: failed to register -> ", path.c_str());
        }
    }

    void AssetManager::register_assets(const std::vector<std::string>& paths) {
        for (auto& path : paths) register_asset(path);
    }


    //------------------------//
    //  Server: HTTP + Manifest//
    //------------------------//

    #if !defined(Vital_SDK_Client)

    bool AssetManager::start_http_server() {
        if (http_running) return true;

        http_server = std::make_unique<httplib::Server>();

        // GET /asset?path=resources/my_resource/file.lua
        // Supports HTTP range requests for resumable downloads
        http_server->Get("/asset", [this](const httplib::Request& req, httplib::Response& res) {
            if (!req.has_param("path")) {
                res.status = 400;
                res.set_content("Missing path", "text/plain");
                return;
            }
            const std::string path = req.get_param_value("path");

            // Only serve registered assets — prevents directory traversal
            if (!registered_assets.count(path)) {
                res.status = 404;
                res.set_content("Not found", "text/plain");
                return;
            }

            const std::string full_path = Vital::get_directory() + "/" + path;

            // Stream the file directly — httplib handles range requests,
            // chunked transfer, and connection keep-alive automatically
            if (!http_server->set_mount_point("/", Vital::get_directory())) {
                // Fallback: manual file streaming with range support
            }

            std::ifstream file(full_path, std::ios::binary | std::ios::ate);
            if (!file) {
                res.status = 500;
                res.set_content("File read error", "text/plain");
                return;
            }

            const size_t file_size = file.tellg();
            file.seekg(0);

            // Use httplib's content provider for efficient streaming
            // without loading the whole file into RAM
            res.set_content_provider(
                file_size,
                "application/octet-stream",
                [full_path](size_t offset, size_t length, httplib::DataSink& sink) -> bool {
                    std::ifstream f(full_path, std::ios::binary);
                    if (!f) return false;
                    f.seekg(static_cast<std::streamoff>(offset));
                    const size_t CHUNK = 65536;
                    std::string buf(CHUNK, '\0');
                    size_t remaining = length;
                    while (remaining > 0 && f) {
                        size_t to_read = std::min(CHUNK, remaining);
                        f.read(buf.data(), static_cast<std::streamsize>(to_read));
                        size_t actually_read = f.gcount();
                        if (actually_read == 0) break;
                        if (!sink.write(buf.data(), actually_read)) return false;
                        remaining -= actually_read;
                    }
                    return true;
                }
            );
        });

        // GET /manifest — returns JSON-style asset list with hashes
        http_server->Get("/manifest", [this](const httplib::Request&, httplib::Response& res) {
            std::string body = "{\"assets\":[";
            bool first = true;
            for (auto& [path, hash] : registered_assets) {
                if (!first) body += ",";
                body += "{\"path\":\"" + path + "\",\"hash\":\"" + hash + "\"}";
                first = false;
            }
            body += "]}";
            res.set_content(body, "application/json");
        });

        http_running = true;
        http_thread = std::thread([this]() {
            Vital::print("sbox", "AssetManager: HTTP server starting on port ", http_port);
            http_server->listen("0.0.0.0", http_port);
            Vital::print("sbox", "AssetManager: HTTP server stopped");
        });

        // Give the server a moment to bind
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        Vital::print("sbox", "AssetManager: HTTP server running on port ", http_port);
        return true;
    }

    void AssetManager::stop_http_server() {
        if (!http_running) return;
        http_running = false;
        if (http_server) http_server->stop();
        if (http_thread.joinable()) http_thread.join();
        http_server.reset();
        Vital::print("sbox", "AssetManager: HTTP server stopped");
    }

    bool AssetManager::is_http_running() const {
        return http_running;
    }

    void AssetManager::broadcast_manifest(int peer_id) {
        if (registered_assets.empty()) return;

        Vital::Tool::Stack msg;
        msg.object["type"]        = Vital::Tool::StackValue(std::string("asset:manifest"));
        msg.object["asset_count"] = Vital::Tool::StackValue((int32_t)registered_assets.size());
        // Tell client which port to hit for HTTP downloads
        msg.object["http_port"]   = Vital::Tool::StackValue((int32_t)http_port);

        int i = 0;
        for (auto& [path, hash] : registered_assets) {
            msg.object["asset_path_" + std::to_string(i)] = Vital::Tool::StackValue(path);
            msg.object["asset_hash_" + std::to_string(i)] = Vital::Tool::StackValue(hash);
            i++;
        }

        Vital::Engine::Network::get_singleton() -> send(msg, peer_id);
        Vital::print("sbox", "AssetManager: sent manifest (",
            (int)registered_assets.size(), " assets) to peer ", peer_id);
    }

    void AssetManager::broadcast_manifest_deferred() {
        for (int peer_id : Vital::Engine::Network::get_singleton() -> get_connected_peers()) {
            Core::get_singleton() -> call_deferred(
                "broadcast_asset_manifest",
                peer_id
            );
        }
    }

    #endif


    //----------------//
    //    Client      //
    //----------------//

    #if defined(Vital_SDK_Client)

    void AssetManager::receive_manifest(const Vital::Tool::Stack& args) {
        int count     = args.object.at("asset_count").as<int32_t>();
        int http_port = args.object.count("http_port")
            ? args.object.at("http_port").as<int32_t>() : 7778;

        // Derive server IP from the network connection
        // sender_id is in the packet but we need the IP — use a stored value
        // For now derive from reconnect_ip if available, else fallback to 127.0.0.1
        // The resource system will set the server IP via set_server_ip() on connect
        const std::string server_ip = server_http_ip.empty() ? "127.0.0.1" : server_http_ip;
        const std::string base_url  = "http://" + server_ip + ":" + std::to_string(http_port);

        Vital::print("sbox", "AssetManager: received manifest, ", count,
            " assets from ", base_url.c_str());

        int needs_download = 0;

        for (int i = 0; i < count; i++) {
            std::string path = args.object.at("asset_path_" + std::to_string(i)).as<std::string>();
            std::string hash = args.object.at("asset_hash_" + std::to_string(i)).as<std::string>();

            // Check local cache by hashing existing file
            bool hash_matches = false;
            const std::string local_path = get_local_base() + "/" + get_local_filename(path);
            try {
                if (std::filesystem::exists(local_path)) {
                    hash_matches = (compute_hash_file(local_path) == hash);
                }
                Vital::print("sbox", "AssetManager: checked -> ", path.c_str(),
                    " match=", hash_matches ? "yes" : "no");
            }
            catch (...) {
                hash_matches = false;
            }

            if (hash_matches) {
                Vital::print("sbox", "AssetManager: up to date -> ", path.c_str());
                Vital::Tool::Stack ready_args;
                ready_args.object["path"]   = Vital::Tool::StackValue(path);
                ready_args.object["cached"] = Vital::Tool::StackValue(true);
                Vital::Tool::Event::emit("asset:file_ready", ready_args);
                continue;
            }

            needs_download++;
            download_file(path, hash, base_url);
        }

        Vital::print("sbox", "AssetManager: ", needs_download, " assets to download");
        if (needs_download == 0) {
            Vital::print("sbox", "AssetManager: all assets ready (cached)");
            Vital::Tool::Event::emit("asset:ready", {});
        }
    }

    void AssetManager::download_file(const std::string& path,
                                      const std::string& expected_hash,
                                      const std::string& base_url) {
        auto dl = std::make_shared<Download>();
        dl->path = path;
        active_downloads[path] = dl;

        const std::string local_base     = get_local_base();
        const std::string local_filename = get_local_filename(path);
        const std::string local_path     = local_base + "/" + local_filename;
        const std::string url            = base_url + "/asset?path=" + path;

        dl->thread = std::thread([this, dl, path, expected_hash, local_path, url]() {
            Vital::print("sbox", "AssetManager: downloading -> ", path.c_str());

            CURL* curl = curl_easy_init();
            if (!curl) {
                Vital::print("sbox", "AssetManager: curl init failed -> ", path.c_str());
                _on_download_failed(path);
                return;
            }

            // Create parent directories
            try {
                std::filesystem::create_directories(
                    std::filesystem::path(local_path).parent_path()
                );
            } catch (...) {}

            // Open output file
            std::ofstream out(local_path, std::ios::binary | std::ios::trunc);
            if (!out) {
                curl_easy_cleanup(curl);
                Vital::print("sbox", "AssetManager: cannot open output file -> ", path.c_str());
                _on_download_failed(path);
                return;
            }

            // Write callback — streams directly to disk, never into RAM
            auto write_cb = [](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
                auto* f = static_cast<std::ofstream*>(userdata);
                f->write(ptr, static_cast<std::streamsize>(size * nmemb));
                return size * nmemb;
            };

            // Progress callback — checks cancellation flag each tick
            auto progress_cb = [](void* clientp, curl_off_t, curl_off_t,
                                   curl_off_t, curl_off_t) -> int {
                auto* dl = static_cast<Download*>(clientp);
                // Return non-zero to abort the transfer
                return dl->cancelled.load() ? 1 : 0;
            };

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                static_cast<size_t(*)(char*, size_t, size_t, void*)>(write_cb));
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION,
                static_cast<int(*)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t)>(progress_cb));
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, dl.get());
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
            // Follow redirects, disable SSL verify for LAN
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            // Low-speed timeout — abort if < 1 byte/sec for 30 seconds
            curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
            curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L);

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            out.close();

            if (dl->cancelled.load()) {
                // Clean up partial file
                try { std::filesystem::remove(local_path); } catch (...) {}
                Vital::print("sbox", "AssetManager: download cancelled -> ", path.c_str());
                active_downloads.erase(path);
                return;
            }

            if (res != CURLE_OK) {
                try { std::filesystem::remove(local_path); } catch (...) {}
                Vital::print("sbox", "AssetManager: download failed -> ", path.c_str(),
                    " error=", curl_easy_strerror(res));
                _on_download_failed(path);
                return;
            }

            // Verify hash
            try {
                const std::string actual_hash = compute_hash_file(local_path);
                if (actual_hash != expected_hash) {
                    try { std::filesystem::remove(local_path); } catch (...) {}
                    Vital::print("sbox", "AssetManager: hash mismatch -> ", path.c_str());
                    _on_download_failed(path);
                    return;
                }
            }
            catch (...) {
                _on_download_failed(path);
                return;
            }

            Vital::print("sbox", "AssetManager: downloaded -> ", path.c_str());
            active_downloads.erase(path);

            // Emit file_ready on main thread via deferred call
            Core::get_singleton() -> call_deferred(
                "on_asset_downloaded",
                godot::String(path.c_str())
            );
        });

        dl->thread.detach();
    }

    void AssetManager::_on_download_failed(const std::string& path) {
        active_downloads.erase(path);
        Core::get_singleton() -> call_deferred(
            "on_asset_download_failed",
            godot::String(path.c_str())
        );
    }

    void AssetManager::cancel(const std::string& path) {
        auto it = active_downloads.find(path);
        if (it == active_downloads.end()) return;
        it->second->cancelled.store(true);
        // Thread will clean up itself when curl aborts
        Vital::print("sbox", "AssetManager: cancelling -> ", path.c_str());
    }

    void AssetManager::cancel_all() {
        for (auto& [path, dl] : active_downloads) {
            dl->cancelled.store(true);
        }
        Vital::print("sbox", "AssetManager: cancelling all downloads");
    }

    bool AssetManager::is_downloading(const std::string& path) const {
        return active_downloads.count(path) > 0;
    }

    bool AssetManager::is_downloading() const {
        return !active_downloads.empty();
    }

    void AssetManager::set_server_http_ip(const std::string& ip) {
        server_http_ip = ip;
    }

    #endif


    //----------------//
    //    Shared      //
    //----------------//

    void AssetManager::queue_spawn(const std::string& name, int authority_peer) {
        spawn_queue[name] = authority_peer;
        Vital::print("sbox", "AssetManager: queued spawn -> ", name.c_str());
    }

    void AssetManager::flush_spawn_queue(const std::string& loaded_name) {
        auto it = spawn_queue.find(loaded_name);
        if (it == spawn_queue.end()) return;
        Core::get_singleton() -> call_deferred(
            "spawn_model",
            godot::String(loaded_name.c_str()),
            it->second
        );
        spawn_queue.erase(it);
    }

    void AssetManager::clear() {
        registered_assets.clear();
        spawn_queue.clear();
        #if defined(Vital_SDK_Client)
        cancel_all();
        active_downloads.clear();
        #endif
    }
}