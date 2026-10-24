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
    class Asset : public godot::Node, public Tool::Base<Asset> {
        friend class Tool::Base<Asset>;
        public:
            static constexpr const char* Name = "Asset.manager";
        private:
            struct AssetEntry {
                std::string hash;
                std::string group = "";
            };

            #if defined(VSDK_Client)
            struct Download {
                std::string path;
                std::unordered_set<std::string> groups;
                std::atomic<bool> cancelled { false };
                std::unordered_map<std::string, uint32_t> gens;
            };

            struct PendingSpawn {
                godot::ObjectID placeholder;
                int authority_peer = 1;
            };
            #endif

            mutable std::shared_mutex registered_assets_mutex;
            std::unordered_map<std::string, AssetEntry> registered_assets;
            #if defined(VSDK_Client)
            std::unordered_map<std::string, std::shared_ptr<Download>> active_downloads;
            std::unordered_map<std::string, std::vector<PendingSpawn>> spawn_queue;
            std::unordered_map<std::string, int> group_pending_counts;
            std::unordered_map<std::string, uint32_t> group_generations;
            std::string server_http_ip;
            std::string server_id;
            uint32_t spawn_generation = 0;
            static constexpr int max_downloads = 4;
            static constexpr size_t max_asset_size = 1024ull * 1024 * 1024;
            std::deque<std::function<void()>> download_queue;
            int running_downloads = 0;
            std::shared_ptr<bool> lifetime = std::make_shared<bool>(true);
            void download_file(const std::string& path, const std::string& expected_hash, const std::string& base_url, const std::string& group);
            void pump_downloads();
            void _on_file_ready(const std::string& path, const std::string& group);
            #else
            Tool::HTTP::Server http_server;
            std::unordered_set<int> pending_manifest_peers;
            #endif


            // Instantiators //
            Asset();
            ~Asset();

            
            // Helpers //
            static std::string hash_file(const std::string& path);
        public:
            // Managers //
            void init();
            void clear();


            // Config //
            #if !defined(VSDK_Client)
            void set_http_port(int port);
            int get_http_port() const;
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
            void set_server_id(const std::string& ip, int port);
            std::string get_resources_root() const;
            void cancel(const std::string& path, const std::string& group = "");
            void cancel_group(const std::string& group);
            void cancel_all();
            bool is_downloading(const std::string& path) const;
            bool is_downloading() const;
            bool is_group_pending(const std::string& group) const;
            // TODO: REMOVE IT? or move under Model?
            void queue_spawn(const std::string& name, godot::ObjectID placeholder, int authority_peer = 1);
            void flush_spawn_queue(const std::string& loaded_name);
            // Drop pending placeholders (e.g. resource stop / unload).
            void clear_spawn_queue(const std::string& loaded_name);
            void clear_spawn_queue_prefix(const std::string& prefix);
            // Hydrate every queued name that is already in Model::cache_loaded.
            void flush_ready_spawns();
            #endif
    };
}