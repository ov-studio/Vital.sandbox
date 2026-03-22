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

#pragma once
#include <Vital.extension/Engine/public/asset.h>
#include <Vital.extension/Engine/public/model.h>
#include <Vital.extension/Engine/public/console.h>
#include <Vital.sandbox/Tool/crypto.h>
#include <Vital.sandbox/Tool/shrinker.h>


/////////////////////////////////////
// Vital: Engine: Asset Manager   //
/////////////////////////////////////

namespace Vital::Engine {

    //TODO: Improve
    AssetManager* AssetManager::get_singleton() {
        if (!singleton) singleton = new AssetManager();
        return singleton;
    }

    void AssetManager::free_singleton() {
        if (!singleton) return;
        delete singleton;
        singleton = nullptr;
    }


    //----------------//
    //    Helpers     //
    //----------------//

    std::string AssetManager::compute_hash(const godot::PackedByteArray& buffer) {
        std::string_view data(reinterpret_cast<const char*>(buffer.ptr()), buffer.size());
        return Vital::Tool::Crypto::hash("SHA256", data);
    }

    std::string AssetManager::get_local_filename(const std::string& path) const {
        std::string flat = path;
        std::replace(flat.begin(), flat.end(), '/', '_');
        std::replace(flat.begin(), flat.end(), '\\', '_');
        return flat;
    }

    std::string AssetManager::get_local_base() const {
        return get_directory() + "/" + output_directory;
    }


    //----------------//
    //    Config      //
    //----------------//

    void AssetManager::set_output_directory(const std::string& dir) {
        output_directory = dir;
        while (!output_directory.empty() && output_directory.back() == '/') {
            output_directory.pop_back();
        }
    }

    const std::string& AssetManager::get_output_directory() const {
        return output_directory;
    }


    //----------------//
    //    Server      //
    //----------------//

    void AssetManager::register_asset(const std::string& path) {
        try {
            auto buffer = Vital::Tool::File::read_binary(get_directory(), path);
            registered_assets[path] = compute_hash(buffer);
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

    void AssetManager::broadcast_manifest(int peer_id) {
        Vital::print("sbox", "AssetManager: broadcast_manifest start, peer=", peer_id);
        if (registered_assets.empty()) return;

        Vital::Tool::Stack msg;
        msg.object["type"]        = Vital::Tool::StackValue(std::string("asset:manifest"));
        msg.object["asset_count"] = Vital::Tool::StackValue((int32_t)registered_assets.size());

        int i = 0;
        for (auto& [path, hash] : registered_assets) {
            msg.object["asset_path_" + std::to_string(i)] = Vital::Tool::StackValue(path);
            msg.object["asset_hash_" + std::to_string(i)] = Vital::Tool::StackValue(hash);
            i++;
        }

        Vital::Engine::Network::get_singleton()->send(msg, peer_id);
        Vital::print("sbox", "AssetManager: sent manifest (",
            (int)registered_assets.size(), " assets) to peer ", peer_id);
    }

    void AssetManager::send_asset(const std::string& path, int peer_id) {
        Vital::print("sbox", "AssetManager: send_asset called -> ", path.c_str(), " peer=", peer_id);

        auto it = registered_assets.find(path);
        if (it == registered_assets.end()) {
            Vital::print("sbox", "AssetManager: asset not registered -> ", path.c_str());
            return;
        }

        try {
            auto buffer = Vital::Tool::File::read_binary(get_directory(), path);
            Vital::print("sbox", "AssetManager: read file ok, size=", (int)buffer.size());

            // Compress then base64 encode
            std::string_view raw(reinterpret_cast<const char*>(buffer.ptr()), buffer.size());
            std::string compressed = Vital::Tool::Shrinker::compress(std::string(raw));
            std::string encoded    = Vital::Tool::Crypto::encode(compressed);
            Vital::print("sbox", "AssetManager: compressed+encoded ok, size=", (int)encoded.size());

            // Split into 256KB chunks
            const size_t CHUNK_SIZE = 262144;
            int total_chunks = (int)std::ceil((double)encoded.size() / CHUNK_SIZE);

            for (int chunk_index = 0; chunk_index < total_chunks; chunk_index++) {
                size_t offset     = chunk_index * CHUNK_SIZE;
                size_t length     = std::min(CHUNK_SIZE, encoded.size() - offset);
                std::string chunk = encoded.substr(offset, length);

                Vital::Tool::Stack msg;
                msg.object["type"]        = Vital::Tool::StackValue(std::string("asset:chunk"));
                msg.object["path"]        = Vital::Tool::StackValue(path);
                msg.object["hash"]        = Vital::Tool::StackValue(it->second);
                msg.object["data"]        = Vital::Tool::StackValue(chunk);
                msg.object["chunk_index"] = Vital::Tool::StackValue((int32_t)chunk_index);
                msg.object["chunk_total"] = Vital::Tool::StackValue((int32_t)total_chunks);

                Vital::Engine::Network::get_singleton()->send(msg, peer_id);
            }

            Vital::print("sbox", "AssetManager: sent ", total_chunks,
                " chunks for -> ", path.c_str(), " to peer ", peer_id);
        }
        catch (...) {
            Vital::print("sbox", "AssetManager: failed to send -> ", path.c_str());
        }
    }


    //----------------//
    //    Client      //
    //----------------//

    void AssetManager::receive_manifest(const Vital::Tool::Stack& args) {
        int count = args.object.at("asset_count").as<int32_t>();
        Vital::print("sbox", "AssetManager: received manifest, ", count, " assets");

        int needs_download = 0;

        for (int i = 0; i < count; i++) {
            std::string path = args.object.at("asset_path_" + std::to_string(i)).as<std::string>();
            std::string hash = args.object.at("asset_hash_" + std::to_string(i)).as<std::string>();

            // Recompute hash from actual file on disk — tamper proof
            bool hash_matches = false;
            try {
                auto buffer = Vital::Tool::File::read_binary(
                    get_local_base(),
                    get_local_filename(path)
                );
                std::string local_hash = compute_hash(buffer);
                hash_matches = (local_hash == hash);
                Vital::print("sbox", "AssetManager: checked -> ", path.c_str(),
                    " match=", hash_matches ? "yes" : "no");
            }
            catch (...) {
                // File doesn't exist or can't be read — needs download
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

            downloading.insert(path);
            needs_download++;

            Vital::Tool::Stack req;
            req.object["type"] = Vital::Tool::StackValue(std::string("asset:request"));
            req.object["path"] = Vital::Tool::StackValue(path);
            Vital::Engine::Network::get_singleton()->send_to_server(req);
            Vital::print("sbox", "AssetManager: requesting -> ", path.c_str());
        }

        Vital::print("sbox", "AssetManager: ", needs_download, " assets to download");
        if (needs_download == 0) {
            Vital::print("sbox", "AssetManager: all assets ready (cached)");
            Vital::Tool::Event::emit("asset:ready", {});
        }
    }

    void AssetManager::receive_chunk(const Vital::Tool::Stack& args) {
        std::string path       = args.object.at("path").as<std::string>();
        std::string hash       = args.object.at("hash").as<std::string>();
        std::string chunk_data = args.object.at("data").as<std::string>();
        int chunk_index        = args.object.at("chunk_index").as<int32_t>();
        int chunk_total        = args.object.at("chunk_total").as<int32_t>();

        Vital::print("sbox", "AssetManager: received chunk ",
            chunk_index + 1, "/", chunk_total, " -> ", path.c_str());

        // Accumulate base64 chunks
        chunk_accumulator[path] += chunk_data;

        // Wait until all chunks arrived
        if (chunk_index < chunk_total - 1) return;

        // All chunks received — decode then decompress
        std::string encoded = chunk_accumulator[path];
        chunk_accumulator.erase(path);

        std::string decoded;
        std::string decompressed;

        try {
            decoded      = Vital::Tool::Crypto::decode(encoded);
            decompressed = Vital::Tool::Shrinker::decompress(decoded);
        }
        catch (...) {
            Vital::print("sbox", "AssetManager: failed to decode/decompress -> ", path.c_str());
            return;
        }

        if (decompressed.empty()) {
            Vital::print("sbox", "AssetManager: empty after decompress -> ", path.c_str());
            return;
        }

        godot::PackedByteArray buffer;
        buffer.resize(decompressed.size());
        memcpy(buffer.ptrw(), decompressed.data(), decompressed.size());

        // Verify hash against original uncompressed data
        std::string received_hash = compute_hash(buffer);
        if (received_hash != hash) {
            Vital::print("sbox", "AssetManager: hash mismatch -> ", path.c_str());
            Vital::Tool::Stack req;
            req.object["type"] = Vital::Tool::StackValue(std::string("asset:request"));
            req.object["path"] = Vital::Tool::StackValue(path);
            Vital::Engine::Network::get_singleton()->send_to_server(req);
            return;
        }

        // Queue heavy work for next frame — keeps network receive non-blocking
        pending_chunks[path] = { buffer, hash };
        Core::get_singleton()->call_deferred(
            "process_asset_chunk",
            godot::String(path.c_str())
        );
    }

    void AssetManager::process_chunk(const std::string& path) {
        auto it = pending_chunks.find(path);
        if (it == pending_chunks.end()) return;

        auto& [buffer, hash] = it->second;
        std::string local_base = get_local_base();

        // Save binary file
        try {
            Vital::Tool::File::write_binary(
                local_base,
                get_local_filename(path),
                buffer
            );
            Vital::print("sbox", "AssetManager: saved -> ", get_local_filename(path).c_str());
        }
        catch (...) {
            Vital::print("sbox", "AssetManager: failed to save -> ", path.c_str());
        }

        Vital::print("sbox", "AssetManager: downloaded -> ", path.c_str());

        // Emit per-file event — caller decides what to do with it
        Vital::Tool::Stack ready_args;
        ready_args.object["path"]   = Vital::Tool::StackValue(path);
        ready_args.object["cached"] = Vital::Tool::StackValue(false);
        Vital::Tool::Event::emit("asset:file_ready", ready_args);

        pending_chunks.erase(it);
        downloading.erase(path);

        if (downloading.empty()) {
            Vital::print("sbox", "AssetManager: all assets ready");
            Vital::Tool::Event::emit("asset:ready", {});
        }
    }

    void AssetManager::queue_spawn(const std::string& name, int authority_peer) {
        spawn_queue[name] = authority_peer;
        Vital::print("sbox", "AssetManager: queued spawn -> ", name.c_str());
    }

    void AssetManager::flush_spawn_queue(const std::string& loaded_name) {
        auto it = spawn_queue.find(loaded_name);
        if (it == spawn_queue.end()) return;
        Vital::print("sbox", "AssetManager: flushing queued spawn -> ", loaded_name.c_str());
        Core::get_singleton()->call_deferred(
            "spawn_model",
            godot::String(loaded_name.c_str()),
            it->second
        );
        spawn_queue.erase(it);
    }

    bool AssetManager::is_downloading() const {
        return !downloading.empty();
    }

    int AssetManager::get_pending_count() const {
        return static_cast<int>(downloading.size());
    }


    //----------------//
    //    Shared      //
    //----------------//

    void AssetManager::clear() {
        registered_assets.clear();
        spawn_queue.clear();
        downloading.clear();
        pending_chunks.clear();
        chunk_accumulator.clear();
    }
}