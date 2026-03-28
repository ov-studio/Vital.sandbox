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
        // Extract base directory and filename using Vital::Tool::File::hash path format
        size_t last_sep = full_path.find_last_of("/\\");
        std::string base = (last_sep != std::string::npos) ? full_path.substr(0, last_sep) : ".";
        std::string file = (last_sep != std::string::npos) ? full_path.substr(last_sep + 1) : full_path;
        return Vital::Tool::File::hash(base, file, "SHA256");
    }


    //----------------//
    //    Config      //
    //----------------//

    #if !defined(Vital_SDK_Client)
    void AssetManager::set_http_port(int port) { http_port = port; }
    int  AssetManager::get_http_port() const   { return http_port; }

    void AssetManager::set_server_info(const ServerInfo& info) {
        server_info = info;
        Vital::print("sbox", "Server info set: '", info.name, "' v", info.version);
    }

    const ServerInfo& AssetManager::get_server_info() const {
        return server_info;
    }
    #endif


    //--------------------//
    //  Asset Registration//
    //--------------------//

    void AssetManager::register_asset(const std::string& path, const std::string& group) {
        // If already registered, update the group but skip re-hashing —
        // hash is stable, no need to recompute
        if (registered_assets.count(path)) {
            if (!group.empty()) registered_assets[path].group = group;
            return;
        }
        try {
            const std::string full_path = Vital::get_directory() + "/" + path;
            // Hash from disk without loading into memory
            registered_assets[path] = { compute_hash_file(full_path), group };
            Vital::print("sbox", "AssetManager: registered -> ", path.c_str(),
                " hash=", registered_assets[path].hash.c_str(),
                " group=", group.empty() ? "(none)" : group.c_str());
        }
        catch (...) {
            Vital::print("sbox", "AssetManager: failed to register -> ", path.c_str());
        }
    }

    void AssetManager::register_assets(const std::vector<std::string>& paths, const std::string& group) {
        for (auto& path : paths) register_asset(path, group);
    }

    void AssetManager::unregister_asset(const std::string& path) {
        auto it = registered_assets.find(path);
        if (it == registered_assets.end()) return;
        Vital::print("sbox", "AssetManager: unregistered -> ", path.c_str());
        registered_assets.erase(it);
    }

    void AssetManager::unregister_group(const std::string& group) {
        int count = 0;
        for (auto it = registered_assets.begin(); it != registered_assets.end(); ) {
            if (it->second.group == group) {
                Vital::print("sbox", "AssetManager: unregistered -> ", it->first.c_str());
                it = registered_assets.erase(it);
                count++;
            } else {
                ++it;
            }
        }
        Vital::print("sbox", "AssetManager: unregistered group '", group.c_str(),
            "' (", count, " asset(s) removed)");
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

        // GET /manifest — returns JSON-style asset list with hashes and groups
        http_server->Get("/manifest", [this](const httplib::Request&, httplib::Response& res) {
            std::string body = "{\"assets\":[";
            bool first = true;
            for (auto& [path, entry] : registered_assets) {
                if (!first) body += ",";
                body += "{\"path\":\"" + path + "\",\"hash\":\"" + entry.hash + "\"";
                if (!entry.group.empty()) body += ",\"group\":\"" + entry.group + "\"";
                body += "}";
                first = false;
            }
            body += "]}";
            res.set_content(body, "application/json");
        });

        // GET /info — returns server info from config
        // This allows clients to query server name, version, social links, etc.
        http_server->Get("/info", [this](const httplib::Request&, httplib::Response& res) {
            rapidjson::Document document;
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.SetObject();
            auto& alloc = document.GetAllocator();
            document.AddMember("name", rapidjson::Value(server_info.name.c_str(), alloc), alloc);
            document.AddMember("version", rapidjson::Value(server_info.version.c_str(), alloc), alloc);
            document.AddMember("description", rapidjson::Value(server_info.description.c_str(), alloc), alloc);
            document.AddMember("http_port", http_port, alloc);
            document.AddMember("max_clients", server_info.max_clients, alloc);
            document.AddMember("discord_invite", rapidjson::Value(server_info.discord.c_str(), alloc), alloc);
            document.AddMember("website", rapidjson::Value(server_info.website.c_str(), alloc), alloc);
            document.Accept(writer);
            res.set_content(buffer.GetString(), "application/json");
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
        for (auto& [path, entry] : registered_assets) {
            msg.object["asset_path_"  + std::to_string(i)] = Vital::Tool::StackValue(path);
            msg.object["asset_hash_"  + std::to_string(i)] = Vital::Tool::StackValue(entry.hash);
            msg.object["asset_group_" + std::to_string(i)] = Vital::Tool::StackValue(entry.group);
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
            std::string path  = args.object.at("asset_path_"  + std::to_string(i)).as<std::string>();
            std::string hash  = args.object.at("asset_hash_"  + std::to_string(i)).as<std::string>();
            // Group is optional — older servers won't send it
            std::string group = args.object.count("asset_group_" + std::to_string(i))
                ? args.object.at("asset_group_" + std::to_string(i)).as<std::string>() : "";

            // Check local cache by hashing existing file
            bool hash_matches = false;
            const std::string local_path = get_directory() + "/" + path;
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
            download_file(path, hash, base_url, group);
        }

        Vital::print("sbox", "AssetManager: ", needs_download, " assets to download");
        if (needs_download == 0) {
            Vital::print("sbox", "AssetManager: all assets ready (cached)");
            Vital::Tool::Event::emit("asset:ready", {});
        }
    }

    void AssetManager::download_file(const std::string& path,
                                      const std::string& expected_hash,
                                      const std::string& base_url,
                                      const std::string& group) {
        auto dl = std::make_shared<Download>();
        dl->path  = path;
        dl->group = group;
        active_downloads[path] = dl;

        const std::string local_base     = get_directory();
        const std::string local_path     = local_base + "/" + path;

        dl->thread = std::thread([this, dl, path, expected_hash, base_url, local_path]() {
            Vital::print("sbox", "AssetManager: downloading -> ", path.c_str());

            // Create parent directories
            try {
                std::filesystem::create_directories(
                    std::filesystem::path(local_path).parent_path()
                );
            } catch (...) {}

            // Open output file
            std::ofstream out(local_path, std::ios::binary | std::ios::trunc);
            if (!out) {
                Vital::print("sbox", "AssetManager: cannot open output file -> ", path.c_str());
                _on_download_failed(path);
                return;
            }

            // Use Vital::Tool::Rest::get for downloading
            std::string response_body;
            try {
                response_body = Vital::Tool::Rest::get(base_url + "/asset?path=" + path);
            } catch (const std::exception& e) {
                Vital::print("sbox", "AssetManager: download failed -> ", path.c_str(),
                    " error=", e.what());
                _on_download_failed(path);
                return;
            }

            // Write response to file
            out.write(response_body.data(), static_cast<std::streamsize>(response_body.size()));
            out.close();

            if (dl->cancelled.load()) {
                try { std::filesystem::remove(local_path); } catch (...) {}
                Vital::print("sbox", "AssetManager: download cancelled -> ", path.c_str());
                active_downloads.erase(path);
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
        // Thread will clean up itself when it polls and sees cancelled flag
        Vital::print("sbox", "AssetManager: cancelling -> ", path.c_str());
    }

    void AssetManager::cancel_group(const std::string& group) {
        int cancelled_count = 0;
        for (auto& [path, dl] : active_downloads) {
            if (dl->group == group) {
                dl->cancelled.store(true);
                cancelled_count++;
            }
        }
        Vital::print("sbox", "AssetManager: cancelling group '", group.c_str(),
            "' (", cancelled_count, " download(s) flagged)");
    }

    void AssetManager::cancel_all() {
        for (auto& [path, dl] : active_downloads) {
            dl->cancelled.store(true);
        }
        Vital::print("sbox", "AssetManager: cancelling all downloads (threads will clean up on next poll)");
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