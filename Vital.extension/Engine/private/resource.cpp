/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: resource.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Resource Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/resource.h>
#include <Vital.extension/Engine/public/console.h>
#include <yaml-cpp/yaml.h>


//////////////////////////////
// Vital: Engine: Resource //
//////////////////////////////

namespace Vital::Engine {
    static const std::unordered_set<std::string> valid_script_types = { "server", "client", "shared" };


    // Utils //
    ResourceManager* ResourceManager::get_singleton() {
        if (!singleton) singleton = new ResourceManager();
        return singleton;
    }

    void ResourceManager::free_singleton() {
        if (!singleton) return;
        delete singleton;
        singleton = nullptr;
    }


    // APIs //
    void ResourceManager::scan() {
        Vital::print("sbox", "Rescanning resources...");
        resources.clear();

        const std::string base = Vital::get_directory();

        // Scan all subdirectories inside resources/
        std::vector<std::string> folders;
        try {
            folders = Vital::Tool::File::contents(base, "resources", true);
        }
        catch (...) {
            Vital::print("error", "Resource scan skipped — `resources/` directory not found");
            return;
        }

        // First pass — detect duplicates by folder name only
        std::unordered_map<std::string, int> folder_count;
        for (const auto& folder_path : folders) {
            const std::string folder = folder_path.substr(folder_path.find_last_of("/\\") + 1);
            folder_count[folder]++;
        }

        // Second pass — load valid resources
        for (const auto& folder_path : folders) {
            const std::string folder = folder_path.substr(folder_path.find_last_of("/\\") + 1);

            // Duplicate check
            if (folder_count[folder] > 1) {
                Vital::print("error", "Duplicate resource found — skipping `" + folder + "`");
                continue;
            }

            // Manifest existence check
            if (!Vital::Tool::File::exists(base, folder_path + "/manifest.yaml")) continue;

            // Read and parse manifest
            YAML::Node manifest;
            try {
                const std::string content = Vital::Tool::File::read_text(base, folder_path + "/manifest.yaml");
                manifest = YAML::Load(content);
            }
            catch (const YAML::Exception& e) {
                Vital::print("error", "Failed to parse manifest for `" + folder + "` — " + e.what());
                continue;
            }
            catch (...) {
                Vital::print("error", "Failed to read manifest for `" + folder + "`");
                continue;
            }

            ResourceManifest res;
            res.folder  = folder;
            res.name    = manifest["name"]    ? manifest["name"].as<std::string>()    : folder;
            res.author  = manifest["author"]  ? manifest["author"].as<std::string>()  : "";
            res.version = manifest["version"] ? manifest["version"].as<std::string>() : "";

            // Scripts — required
            if (!manifest["scripts"] || !manifest["scripts"].IsSequence()) {
                Vital::print("error", "Resource `" + folder + "` has no valid `scripts` section — skipping");
                continue;
            }

            // Validate script entries and existence
            bool valid = true;
            for (const auto& node : manifest["scripts"]) {
                if (!node["src"] || !node["type"]) {
                    Vital::print("error", "Resource `" + folder + "` has a script entry missing `src` or `type` — skipping resource");
                    valid = false;
                    break;
                }
                const std::string type = node["type"].as<std::string>();
                if (valid_script_types.find(type) == valid_script_types.end()) {
                    Vital::print("error", "Resource `" + folder + "` has invalid script type `" + type + "` — skipping resource");
                    valid = false;
                    break;
                }
                const std::string src = node["src"].as<std::string>();
                if (!Vital::Tool::File::exists(base, folder_path + "/" + src)) {
                    Vital::print("error", "Resource `" + folder + "` script `" + src + "` not found on disk — skipping resource");
                    valid = false;
                    break;
                }
                res.scripts.push_back({ src, type });
            }
            if (!valid) continue;

            // Validate and collect files — optional
            if (manifest["files"] && manifest["files"].IsSequence()) {
                for (const auto& node : manifest["files"]) {
                    const std::string file = node.as<std::string>();
                    if (!Vital::Tool::File::exists(base, folder_path + "/" + file)) {
                        Vital::print("error", "Resource `" + folder + "` file `" + file + "` not found on disk — skipping resource");
                        valid = false;
                        break;
                    }
                    res.files.push_back(file);
                }
            }
            if (!valid) continue;

            resources.push_back(std::move(res));
            Vital::print("sbox", "Loaded resource `" + folder + "`");
        }

        Vital::print("sbox", "Resource scan complete — " + std::to_string(resources.size()) + " resource(s) loaded");
    }

    bool ResourceManager::is_loaded(const std::string& folder) const {
        for (const auto& res : resources)
            if (res.folder == folder) return true;
        return false;
    }

    const ResourceManifest* ResourceManager::get_resource(const std::string& folder) const {
        for (const auto& res : resources)
            if (res.folder == folder) return &res;
        return nullptr;
    }

    std::vector<const ResourceManifest*> ResourceManager::get_all_resources() const {
        std::vector<const ResourceManifest*> result;
        result.reserve(resources.size());
        for (const auto& res : resources)
            result.push_back(&res);
        return result;
    }

    std::vector<ResourceScript> ResourceManager::get_scripts(const std::string& folder, const std::string& type) const {
        std::vector<ResourceScript> result;
        const auto* res = get_resource(folder);
        if (!res) return result;
        for (const auto& script : res->scripts)
            if (type.empty() || script.type == type) result.push_back(script);
        return result;
    }
}