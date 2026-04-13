/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: asset.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Asset Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>


////////////////////////////
// Vital: Manager: Asset //
////////////////////////////

// TODO: Improve
namespace Vital::Manager {
    struct ServerInfo {
        std::string name = "Vital Sandbox Server";
        std::string version = "1.0.0";
        std::string description = "";
        int max_clients = 32;
        std::string discord = "";
        std::string website = "";
    };

    class Asset {
        private:
            struct AssetEntry {
                std::string hash;
                std::string group;
            };

            #if defined(Vital_SDK_Client)
            struct Download {
                std::string path;
                std::unordered_set<std::string> groups;
                std::thread thread;
                std::atomic<bool> cancelled{false};
                // How many resources currently need this file.
                // When it drops to 0 mid-download the download is cancelled.
                std::atomic<int> ref_count{1};
            };
            std::unordered_map<std::string, std::shared_ptr<Download>> active_downloads;
            std::string server_http_ip;
            void download_file(const std::string& path, const std::string& expected_hash, const std::string& base_url, const std::string& group);
            void _on_download_failed(const std::string& path);
            #else
            std::unique_ptr<httplib::Server> http_server;
            std::thread http_thread;
            int http_port = 7778;
            bool http_running = false;
            ServerInfo server_info;
            // Peers that already have a deferred broadcast queued — prevents
            // multiple resources starting in the same frame from sending the
            // manifest more than once per peer.
            std::unordered_set<int> pending_manifest_peers;
            #endif

            inline static Asset* singleton = nullptr;
            std::unordered_map<std::string, AssetEntry> registered_assets;
            std::unordered_map<std::string, int> spawn_queue;
            static std::string hash_file(const std::string& path);

        public:
            Asset()  = default;
            ~Asset() = default;

            static Asset* get_singleton();
            static void free_singleton();


            // APIs //
            void init();
            void clear();


            // Config //
            #if !defined(Vital_SDK_Client)
            void set_http_port(int port);
            int  get_http_port() const;
            void set_server_info(const ServerInfo& info);
            const ServerInfo& get_server_info() const;
            #endif


            // Registration //
            void register_asset(const std::string& path, const std::string& group = "", bool silenced = false);
            void register_assets(const std::vector<std::string>& paths, const std::string& group = "");
            void unregister_asset(const std::string& path);
            void unregister_group(const std::string& group);


            // Manifest //
            void broadcast_manifest(int peer_id);
            void broadcast_manifest_deferred();


            // Server //
            #if !defined(Vital_SDK_Client)
            bool start_http_server();
            void stop_http_server();
            bool is_http_running() const;
            #endif


            // Client //
            #if defined(Vital_SDK_Client)
            void receive_manifest(const Tool::Stack& args);
            void set_server_http_ip(const std::string& ip);
            // cancel(path)      — decrements ref-count; cancels only when it hits 0
            // cancel_group(grp) — decrements ref-count of every download belonging to
            //                     the group; cancels those whose count reaches 0
            // cancel_all()      — force-cancels everything regardless of ref-count
            void cancel(const std::string& path);
            void cancel_group(const std::string& group);
            void cancel_all();
            bool is_downloading(const std::string& path) const;
            bool is_downloading() const;
            #endif


            // Shared //
            void queue_spawn(const std::string& name, int authority_peer);
            void flush_spawn_queue(const std::string& loaded_name);
    };
}