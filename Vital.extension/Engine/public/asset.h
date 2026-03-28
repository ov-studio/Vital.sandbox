/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: asset.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Asset Manager Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/index.h>

// Forward declare httplib::Server so the header does not need to include
// httplib.h — the full include lives in asset_manager.cpp only
#if !defined(Vital_SDK_Client)
namespace httplib { class Server; }
#endif


/////////////////////////////////////
// Vital: Engine: Asset Manager   //
/////////////////////////////////////

namespace Vital::Engine {
    // TODO: Improve
    class AssetManager {
        private:
            inline static AssetManager* singleton = nullptr;

            std::string output_directory = "vital_assets";

            // Server + Client: registered assets path → hash
            std::unordered_map<std::string, std::string> registered_assets;

            // Spawn queue — shared between platforms (used by model system)
            std::unordered_map<std::string, int> spawn_queue;

            #if !defined(Vital_SDK_Client)
            // HTTP server runs on a background thread — never blocks Godot
            std::unique_ptr<httplib::Server> http_server;
            std::thread http_thread;
            int http_port = 7778;
            bool http_running = false;
            #endif

            #if defined(Vital_SDK_Client)
            struct Download {
                std::string path;
                std::thread thread;
                std::atomic<bool> cancelled{false};
            };
            std::unordered_map<std::string, std::shared_ptr<Download>> active_downloads;
            std::string server_http_ip;
            void download_file(const std::string& path, const std::string& expected_hash, const std::string& base_url);
            void _on_download_failed(const std::string& path);
            #endif

            // Helpers
            static std::string compute_hash(const godot::PackedByteArray& buffer);
            static std::string compute_hash_file(const std::string& full_path);

        public:
            AssetManager()  = default;
            ~AssetManager() = default;

            static AssetManager* get_singleton();
            static void free_singleton();


            // Config //
            void set_output_directory(const std::string& dir);
            const std::string& get_output_directory() const;
            std::string get_local_filename(const std::string& path) const;
            std::string get_local_base() const;

            #if !defined(Vital_SDK_Client)
            void set_http_port(int port);
            int  get_http_port() const;
            #endif


            // Registration //
            void register_asset(const std::string& path);
            void register_assets(const std::vector<std::string>& paths);


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
            void receive_manifest(const Vital::Tool::Stack& args);
            void set_server_http_ip(const std::string& ip);
            void cancel(const std::string& path);
            void cancel_all();
            bool is_downloading(const std::string& path) const;
            bool is_downloading() const;
            #endif


            // Shared //
            void clear();
            void queue_spawn(const std::string& name, int authority_peer);
            void flush_spawn_queue(const std::string& loaded_name);
    };
}