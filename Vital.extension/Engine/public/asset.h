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
#include <Vital.extension/Engine/public/core.h>
#include <Vital.extension/Engine/public/network.h>
#include <Vital.sandbox/Tool/file.h>
#include <Vital.sandbox/Tool/crypto.h>
#include <Vital.sandbox/Tool/shrinker.h>


/////////////////////////////////////
// Vital: Engine: Asset Manager   //
/////////////////////////////////////

namespace Vital::Engine {
    //TODO: Improve
    class AssetManager {
        private:
            inline static AssetManager* singleton = nullptr;

            // Output directory where downloaded files are saved on client
            std::string output_directory = "vital_assets";

            // Server: registered assets path → hash
            std::unordered_map<std::string, std::string> registered_assets;

            // Client: spawn queue — models waiting for asset name → authority
            std::unordered_map<std::string, int> spawn_queue;

            // Client: assets currently being downloaded
            std::unordered_set<std::string> downloading;

            // Client: pending chunks awaiting process_chunk
            struct ChunkData { godot::PackedByteArray buffer; std::string hash; };
            std::unordered_map<std::string, ChunkData> pending_chunks;

            // Client: chunk accumulator path → accumulated base64 data
            std::unordered_map<std::string, std::string> chunk_accumulator;

            // Helpers //
            static std::string compute_hash(const godot::PackedByteArray& buffer);
            std::string get_local_base() const;

        public:
            AssetManager()  = default;
            ~AssetManager() = default;

            static AssetManager* get_singleton();
            static void free_singleton();


            // Config //
            void set_output_directory(const std::string& dir);
            const std::string& get_output_directory() const;


            // Helpers (public) //
            std::string get_local_filename(const std::string& path) const;


            // Server //
            void register_asset(const std::string& path);
            void register_assets(const std::vector<std::string>& paths);
            void broadcast_manifest(int peer_id);
            void send_asset(const std::string& path, int peer_id);


            // Client //
            void receive_manifest(const Vital::Tool::Stack& args);
            void receive_chunk(const Vital::Tool::Stack& args);
            void process_chunk(const std::string& path);
            void queue_spawn(const std::string& name, int authority_peer);
            void flush_spawn_queue(const std::string& loaded_name);
            bool is_downloading() const;
            int get_pending_count() const;


            // Shared //
            void clear();
    };
}