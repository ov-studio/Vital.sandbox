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
#include <Vital.sandbox/Manager/public/network.h>


////////////////////////////
// Vital: Manager: Asset //
////////////////////////////

// TODO: Improve
namespace Vital::Manager {
    class Asset {
        private:
            struct AssetEntry {
                std::string hash;
                std::string group = "";
            };

            #if defined(VSDK_Client)
            struct Download {
                std::string path;
                std::unordered_set<std::string> groups;
                std::thread thread;
                std::atomic<bool> cancelled { false };
            };

            // Placeholder node waiting to be hydrated keyed by model name.
            // Stored as void* to avoid a circular include with model.h;
            // asset.cpp casts it back to Engine::Model* where needed.
            struct PendingSpawn {
                void* placeholder = nullptr; // Engine::Model*
                int authority_peer = 1;
            };

            std::unordered_map<std::string, std::shared_ptr<Download>> active_downloads;
            std::unordered_map<std::string, PendingSpawn> spawn_queue;
            std::unordered_map<std::string, int> group_pending_counts;
            std::unordered_map<std::string, uint32_t> group_generations;
            std::string server_http_ip;
            void download_file(const std::string& path, const std::string& expected_hash, const std::string& base_url, const std::string& group);
            void _on_download_failed(const std::string& path);
            void _on_file_ready(const std::string& path, const std::string& group);
            #else
            std::unordered_map<std::string, int> spawn_queue; // TODO: unused on server, kept for symmetry, remove it
            std::unique_ptr<httplib::Server> http_server;
            std::thread http_thread;
            int http_port = 7778;
            bool http_running = false;
            std::unordered_set<int> pending_manifest_peers;
            #endif

            inline static Asset* singleton = nullptr;
            std::unordered_map<std::string, AssetEntry> registered_assets;
            static std::string hash_file(const std::string& path);
        public:
            Asset() = default;
            ~Asset() = default;


            // Singleton //
            static Asset* get_singleton();
            static void free_singleton();


            // APIs //
            void init();
            void clear();


            // Config //
            #if !defined(VSDK_Client)
            void set_http_port(int port);
            int  get_http_port() const;
            #endif


            // Registration //
            void register_asset(const std::string& path, const std::string& group = "", bool silenced = false);
            void register_assets(const std::vector<std::string>& paths, const std::string& group = "");
            void unregister_asset(const std::string& path);
            void unregister_group(const std::string& group);
            void broadcast_manifest(int peer_id = -1, bool deferred = false);


            // Server //
            #if !defined(VSDK_Client)
            bool start_http_server();
            void stop_http_server();
            bool is_http_running() const;
            #endif


            // Client //
            #if defined(VSDK_Client)
            void receive_manifest(const Tool::Stack& args);
            void set_server_http_ip(const std::string& ip);
            void cancel(const std::string& path, const std::string& group = "");
            void cancel_group(const std::string& group);
            void cancel_all();
            bool is_downloading(const std::string& path) const;
            bool is_downloading() const;
            bool is_group_pending(const std::string& group) const;
            // TODO: REMOVE IT? or move under Model?
            void queue_spawn(const std::string& name, void* placeholder, int authority_peer = 1);
            void flush_spawn_queue(const std::string& loaded_name);
            #endif
    };
}