/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: asset.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Asset Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#include <Vital.sandbox/Manager/public/asset.h>
#include <Vital.sandbox/Engine/public/network.h>
#include <Vital.sandbox/Engine/public/model.h>
#include <Vital.sandbox/Engine/public/console.h>


////////////////////////////
// Vital: Manager: Asset //
////////////////////////////

// TODO: Improve
namespace Vital::Manager {
    Asset* Asset::get_singleton() {
        if (!singleton) singleton = new Asset();
        return singleton;
    }

    void Asset::free_singleton() {
        if (!singleton) return;
        singleton -> clear();
        #if !defined(Vital_SDK_Client)
        singleton -> stop_http_server();
        #endif
        delete singleton;
        singleton = nullptr;
    }


    //----------------//
    //    Helpers     //
    //----------------//

    std::string Asset::hash_file(const std::string& path) {
        size_t last_sep = path.find_last_of("/\\");
        std::string base = (last_sep != std::string::npos) ? path.substr(0, last_sep) : ".";
        std::string file = (last_sep != std::string::npos) ? path.substr(last_sep + 1) : path;
        return Tool::File::hash(base, file);
    }


    //----------------//
    //     APIs       //
    //----------------//

    void Asset::init() {
        #if defined(Vital_SDK_Client)
            static bool initialized = false;
            if (initialized) return;
            initialized = true;

            Tool::Event::bind("vital.network:packet", [this](Tool::Stack args) {
                if (!args.object.count("event")) return;
                const std::string event = args.object.at("event").as<std::string>();
                if (event != "asset:manifest") return;
                receive_manifest(args);
            });

            Tool::print("sbox", "Asset: initialized");
        #endif
    }

    void Asset::clear() {
        registered_assets.clear();
        spawn_queue.clear();
        #if defined(Vital_SDK_Client)
        cancel_all();
        active_downloads.clear();
        #else
        stop_http_server();
        #endif
    }


    //----------------//
    //    Config      //
    //----------------//

    #if !defined(Vital_SDK_Client)
    void Asset::set_http_port(int port) { http_port = port; }
    int  Asset::get_http_port() const   { return http_port; }

    void Asset::set_server_info(const ServerInfo& info) {
        server_info = info;
        Tool::print("sbox", "Server info set: '", info.name, "' v", info.version);
    }

    const ServerInfo& Asset::get_server_info() const {
        return server_info;
    }
    #endif


    //--------------------//
    //    Registration    //
    //--------------------//

    void Asset::register_asset(const std::string& path, const std::string& group, bool silenced) {
        if (registered_assets.count(path)) {
            if (!group.empty()) registered_assets[path].group = group;
            return;
        }
        try {
            const std::string full_path = Tool::get_directory() + "/" + path;
            registered_assets[path] = { hash_file(full_path), group };
            if (!silenced) {
                std::string report = fmt::format("Asset: registered asset for group `{}`:\n", group.empty() ? "(none)" : group);
                report += fmt::format("> {} — {}", path, registered_assets[path].hash);
                Tool::print("sbox", report);
            }
        }
        catch (...) {
            Tool::print("error", "Asset: failed to register -> ", path.c_str());
        }
    }
    
    void Asset::register_assets(const std::vector<std::string>& paths, const std::string& group) {
        std::vector<std::string> registered;
        for (const auto& path : paths) {
            const size_t before = registered_assets.size();
            register_asset(path, group, true);
            if (registered_assets.size() > before)
                registered.push_back(path);
        }
    
        if (registered.empty()) return;
        std::string report = fmt::format("Asset: registered {} asset(s) for group `{}`:\n", registered.size(), group);
        for (const auto& path : registered)
            report += fmt::format("> {} — {}\n", path, registered_assets[path].hash);
        Tool::print("sbox", report);
    }

    void Asset::unregister_asset(const std::string& path) {
        auto it = registered_assets.find(path);
        if (it == registered_assets.end()) return;
        registered_assets.erase(it);
    }

    void Asset::unregister_group(const std::string& group) {
        int count = 0;
        for (auto it = registered_assets.begin(); it != registered_assets.end(); ) {
            if (it->second.group == group) {
                it = registered_assets.erase(it);
                count++;
            } else {
                ++it;
            }
        }
        Tool::print("sbox", fmt::format("Asset: unregistered group `{}` — {} asset(s) removed", group, count));
    }


    //----------------------------//
    //    Server: HTTP + Manifest //
    //----------------------------//

    #if !defined(Vital_SDK_Client)

    bool Asset::start_http_server() {
        if (http_running) return true;

        http_server = std::make_unique<httplib::Server>();

        http_server->Get("/asset", [this](const httplib::Request& req, httplib::Response& res) {
            if (!req.has_param("path")) {
                res.status = 400;
                res.set_content("Missing path", "text/plain");
                return;
            }
            const std::string path = req.get_param_value("path");

            if (!registered_assets.count(path)) {
                res.status = 404;
                res.set_content("Not found", "text/plain");
                return;
            }

            const std::string full_path = Tool::get_directory() + "/" + path;

            if (!http_server->set_mount_point("/", Tool::get_directory())) {
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

        http_server->Get("/info", [this](const httplib::Request&, httplib::Response& res) {
            rapidjson::Document document;
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.SetObject();
            auto& alloc = document.GetAllocator();
            document.AddMember("name",           rapidjson::Value(server_info.name.c_str(), alloc),        alloc);
            document.AddMember("version",         rapidjson::Value(server_info.version.c_str(), alloc),     alloc);
            document.AddMember("description",     rapidjson::Value(server_info.description.c_str(), alloc), alloc);
            document.AddMember("http_port",       http_port,                                                alloc);
            document.AddMember("max_clients",     server_info.max_clients,                                  alloc);
            document.AddMember("discord_invite",  rapidjson::Value(server_info.discord.c_str(), alloc),     alloc);
            document.AddMember("website",         rapidjson::Value(server_info.website.c_str(), alloc),     alloc);
            document.Accept(writer);
            res.set_content(buffer.GetString(), "application/json");
        });

        http_running = true;
        http_thread = std::thread([this]() {
            Tool::print("sbox", "Asset: HTTP server starting on port ", http_port);
            http_server->listen("0.0.0.0", http_port);
            Tool::print("sbox", "Asset: HTTP server stopped");
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        Tool::print("sbox", "Asset: HTTP server running on port ", http_port);
        return true;
    }

    void Asset::stop_http_server() {
        if (!http_running) return;
        http_running = false;
        if (http_server) http_server->stop();
        if (http_thread.joinable()) http_thread.join();
        http_server.reset();
        Tool::print("sbox", "Asset: HTTP server stopped");
    }

    bool Asset::is_http_running() const {
        return http_running;
    }

    void Asset::broadcast_manifest(int peer_id) {
        if (registered_assets.empty()) return;

        Tool::Stack msg;
        msg.object["event"]        = Tool::StackValue(std::string("asset:manifest"));
        msg.object["asset_count"] = Tool::StackValue((int32_t)registered_assets.size());
        msg.object["http_port"]   = Tool::StackValue((int32_t)http_port);

        int i = 0;
        for (auto& [path, entry] : registered_assets) {
            msg.object["asset_path_"  + std::to_string(i)] = Tool::StackValue(path);
            msg.object["asset_hash_"  + std::to_string(i)] = Tool::StackValue(entry.hash);
            msg.object["asset_group_" + std::to_string(i)] = Tool::StackValue(entry.group);
            i++;
        }

        Engine::Network::get_singleton() -> send(msg, peer_id);
        Tool::print("sbox", "Asset: sent manifest (",
            (int)registered_assets.size(), " assets) to peer ", peer_id);
    }

    // TODO: Needed still??
    void Asset::broadcast_manifest_deferred() {
        for (int peer_id : Engine::Network::get_singleton() -> get_connected_peers()) {
            Engine::Core::get_singleton() -> push_deferred([peer_id]() {
                Asset::get_singleton() -> broadcast_manifest(peer_id);
            });
        }
    }

    #endif


    //----------------//
    //    Client      //
    //----------------//

    #if defined(Vital_SDK_Client)

    void Asset::receive_manifest(const Tool::Stack& args) {
        int count     = args.object.at("asset_count").as<int32_t>();
        int http_port = args.object.count("http_port") ? args.object.at("http_port").as<int32_t>() : 7778;

        const std::string server_ip = server_http_ip.empty() ? "127.0.0.1" : server_http_ip;
        const std::string base_url  = "http://" + server_ip + ":" + std::to_string(http_port);

        Tool::print("sbox", "Asset: received manifest, ", count, " assets from ", base_url.c_str());

        int needs_download = 0;

        for (int i = 0; i < count; i++) {
            std::string path  = args.object.at("asset_path_"  + std::to_string(i)).as<std::string>();
            std::string hash  = args.object.at("asset_hash_"  + std::to_string(i)).as<std::string>();
            std::string group = args.object.count("asset_group_" + std::to_string(i))
                ? args.object.at("asset_group_" + std::to_string(i)).as<std::string>() : "";

            bool hash_matches = false;
            const std::string local_path = Tool::get_directory() + "/" + path;
            try {
                if (std::filesystem::exists(local_path)) hash_matches = (hash_file(local_path) == hash);
                Tool::print("sbox", "Asset: checked -> ", path.c_str(), " match=", hash_matches ? "yes" : "no");
            }
            catch (...) { hash_matches = false; }

            if (hash_matches) {
                Tool::print("sbox", "Asset: up to date -> ", path.c_str());
                Tool::Stack ready_args;
                ready_args.object["path"]   = Tool::StackValue(path);
                ready_args.object["cached"] = Tool::StackValue(true);
                Tool::Event::emit("asset:file_ready", ready_args);
                continue;
            }

            needs_download++;
            download_file(path, hash, base_url, group);
        }

        Tool::print("sbox", "Asset: ", needs_download, " assets to download");
        if (needs_download == 0) {
            Tool::print("sbox", "Asset: all assets ready (cached)");
            Tool::Event::emit("asset:ready", {});
        }
    }

    void Asset::download_file(const std::string& path, const std::string& expected_hash, const std::string& base_url, const std::string& group) {
        auto dl = std::make_shared<Download>();
        dl->path  = path;
        dl->group = group;
        active_downloads[path] = dl;

        const std::string local_base = Tool::get_directory();
        const std::string local_path = local_base + "/" + path;

        dl->thread = std::thread([this, dl, path, expected_hash, base_url, local_path]() {
            Tool::print("sbox", "Asset: downloading -> ", path.c_str());

            try {
                std::filesystem::create_directories(
                    std::filesystem::path(local_path).parent_path()
                );
            } catch (...) {}

            std::ofstream out(local_path, std::ios::binary | std::ios::trunc);
            if (!out) {
                Tool::print("sbox", "Asset: cannot open output file -> ", path.c_str());
                _on_download_failed(path);
                return;
            }

            std::string response_body;
            try {
                response_body = Tool::Rest::get(base_url + "/asset?path=" + path);
            } catch (const std::exception& e) {
                Tool::print("sbox", "Asset: download failed -> ", path.c_str(),
                    " error=", e.what());
            catch (const std::exception& e) {
                _on_download_failed(path);
                return;
            }

            out.write(response_body.data(), static_cast<std::streamsize>(response_body.size()));
            out.close();

            if (dl->cancelled.load()) {
                try { std::filesystem::remove(local_path); } catch (...) {}
                Tool::print("sbox", "Asset: download cancelled -> ", path.c_str());
                active_downloads.erase(path);
                return;
            }

            try {
                const std::string actual_hash = hash_file(local_path);
                if (actual_hash != expected_hash) {
                    try { std::filesystem::remove(local_path); } catch (...) {}
                    Tool::print("sbox", "Asset: hash mismatch -> ", path.c_str());
                    _on_download_failed(path);
                    return;
                }
            }
            catch (...) { _on_download_failed(path); return; }

            Tool::print("sbox", "Asset: downloaded -> ", path.c_str());
            active_downloads.erase(path);

            Engine::Core::get_singleton() -> push_deferred([path]() {
                Tool::Stack ready_args;
                ready_args.object["path"]   = Tool::StackValue(path);
                ready_args.object["cached"] = Tool::StackValue(false);
                Tool::Event::emit("asset:file_ready", ready_args);
                if (!Asset::get_singleton() -> is_downloading()) {
                    Tool::print("sbox", "Asset: all assets ready");
                    Tool::Event::emit("asset:ready", {});
                }
            });
        });

        dl->thread.detach();
    }

    void Asset::_on_download_failed(const std::string& path) {
        active_downloads.erase(path);
        Engine::Core::get_singleton() -> push_deferred([path]() {
            Tool::print("sbox", "Asset: download failed -> ", path);
        });
    }

    void Asset::cancel(const std::string& path) {
        auto it = active_downloads.find(path);
        if (it == active_downloads.end()) return;
        it->second->cancelled.store(true);
        Tool::print("sbox", "Asset: cancelling -> ", path.c_str());
    }

    void Asset::cancel_group(const std::string& group) {
        int cancelled_count = 0;
        for (auto& [path, dl] : active_downloads) {
            if (dl->group == group) {
                dl->cancelled.store(true);
                cancelled_count++;
            }
        }
        Tool::print("sbox", "Asset: cancelling group '", group.c_str(),
            "' (", cancelled_count, " download(s) flagged)");
    }

    void Asset::cancel_all() {
        for (auto& [path, dl] : active_downloads) dl->cancelled.store(true);
        Tool::print("sbox", "Asset: cancelling all downloads");
    }

    bool Asset::is_downloading(const std::string& path) const { return active_downloads.count(path) > 0; }
    bool Asset::is_downloading() const { return !active_downloads.empty(); }
    void Asset::set_server_http_ip(const std::string& ip) { server_http_ip = ip; }

    #endif


    //----------------//
    //    Shared      //
    //----------------//

    void Asset::queue_spawn(const std::string& name, int authority_peer) {
        spawn_queue[name] = authority_peer;
        Tool::print("sbox", "Asset: queued spawn -> ", name.c_str());
    }

    void Asset::flush_spawn_queue(const std::string& loaded_name) {
        auto it = spawn_queue.find(loaded_name);
        if (it == spawn_queue.end()) return;
        const int authority_peer = it->second;
        spawn_queue.erase(it);
        Engine::Core::get_singleton() -> push_deferred([loaded_name, authority_peer]() {
            Engine::Model::spawn_synced(loaded_name, authority_peer);
        });
    }
}