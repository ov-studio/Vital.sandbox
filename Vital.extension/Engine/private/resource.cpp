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

#include <Vital.extension/Engine/public/resource.h>
#include <Vital.extension/Engine/public/console.h>
#include <Vital.extension/Sandbox/index.h>
#include <yaml-cpp/yaml.h>


//////////////////////////////
// Vital: Engine: Resource //
//////////////////////////////

namespace Vital::Engine {
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

    std::string ResourceManager::env_name(const std::string& folder) {
        return folder + ":env";
    }

    bool ResourceManager::is_eligible(const std::string& type) {
        if (type == "shared") return true;
        return type == Vital::get_platform();
    }

    std::string ResourceManager::resource_path(const std::string& folder) {
        return Vital::get_directory("resources", folder);
    }


    // APIs //
    void ResourceManager::scan() {
        Vital::print("sbox", "Rescanning resources...");
        resources.clear();
        const std::string base = Vital::get_directory();
        std::vector<std::string> folders;

        try {
            folders = Vital::Tool::File::contents(base, "resources", true);
        }
        catch (...) {
            Vital::print("error", "Resource scan skipped — `resources/` directory not found");
            return;
        }

        std::unordered_map<std::string, int> folder_count;
        for (const auto& folder_path : folders) {
            const std::string folder = folder_path.substr(folder_path.find_last_of("/\\") + 1);
            folder_count[folder]++;
        }

        for (const auto& folder_path : folders) {
            const std::string folder = folder_path.substr(folder_path.find_last_of("/\\") + 1);
            if (folder_count[folder] > 1) {
                Vital::print("error", "Duplicate resource found — skipping `" + folder + "`");
                continue;
            }
            if (!Vital::Tool::File::exists(resource_path(folder), "manifest.yaml")) continue;

            YAML::Node manifest;
            try {
                const std::string content = Vital::Tool::File::read_text(resource_path(folder), "manifest.yaml");
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
            res.folder = folder;
            res.name = manifest["name"]    ? manifest["name"].as<std::string>()    : folder;
            res.author = manifest["author"]  ? manifest["author"].as<std::string>()  : "";
            res.version = manifest["version"] ? manifest["version"].as<std::string>() : "";
            if (!manifest["scripts"] || !manifest["scripts"].IsSequence()) {
                Vital::print("error", "Resource `" + folder + "` has no valid `scripts` section — skipping");
                continue;
            }

            bool valid = true;
            std::vector<std::string> errors;
            for (const auto& node : manifest["scripts"]) {
                if (!node["src"] || !node["type"]) {
                    errors.push_back("script entry missing `src` or `type`");
                    valid = false;
                    continue;
                }
                const std::string type = node["type"].as<std::string>();
                if (valid_script_types.find(type) == valid_script_types.end()) {
                    errors.push_back("script `" + node["src"].as<std::string>() + "` has invalid type `" + type + "`");
                    valid = false;
                    continue;
                }
                const std::string src = node["src"].as<std::string>();
                if (!Vital::Tool::File::exists(resource_path(folder), src)) {
                    errors.push_back("script `" + src + "` not found on disk");
                    valid = false;
                    continue;
                }
                res.scripts.push_back({ src, type });
            }

            if (manifest["files"] && manifest["files"].IsSequence()) {
                for (const auto& node : manifest["files"]) {
                    const std::string file = node.as<std::string>();
                    if (!Vital::Tool::File::exists(resource_path(folder), file)) {
                        errors.push_back("file `" + file + "` not found on disk");
                        valid = false;
                        continue;
                    }
                    res.files.push_back(file);
                }
            }

            if (!valid) {
                std::string error_list = "Resource `" + folder + "` skipped — " + std::to_string(errors.size()) + " error(s):\n";
                for (const auto& err : errors) {
                    error_list += "> " + err + "\n";
                }
                Vital::print("error", error_list);
                continue;
            }
            resources.push_back(std::move(res));
            Vital::print("sbox", "Loaded resource `" + folder + "`");
        }

        Vital::print("sbox", "Resource scan complete — " + std::to_string(resources.size()) + " resource(s) loaded");
    }

    bool ResourceManager::is_loaded(const std::string& folder) const {
        for (const auto& res : resources) {
            if (res.folder == folder) {
                return true;
            }
        }
        return false;
    }

    bool ResourceManager::is_running(const std::string& folder) const {
        return running.count(folder) > 0;
    }

    const ResourceManager::ResourceManifest* ResourceManager::get_resource(const std::string& folder) const {
        for (const auto& res : resources) {
            if (res.folder == folder) {
                return &res;
            }
        }
        return nullptr;
    }

    std::vector<const ResourceManager::ResourceManifest*> ResourceManager::get_all_resources() const {
        std::vector<const ResourceManager::ResourceManifest*> result;
        result.reserve(resources.size());
        for (const auto& res : resources) {
            result.push_back(&res);
        }
        return result;
    }

    std::vector<ResourceManager::ResourceScript> ResourceManager::get_scripts(const std::string& folder, const std::string& type) const {
        std::vector<ResourceManager::ResourceScript> result;
        const auto* res = get_resource(folder);
        if (!res) return result;
        for (const auto& script : res->scripts) {
            if (type.empty() || script.type == type) {
                result.push_back(script);
            }
        }
        return result;
    }


    // Lifecycle //
    bool ResourceManager::start(const std::string& folder) {
        auto* vm = Sandbox::get_singleton() -> get_vm();
        if (!is_loaded(folder)) {
            Vital::print("error", "Cannot start `" + folder + "` — resource not loaded");
            return false;
        }
        if (is_running(folder)) {
            Vital::print("error", "Cannot start `" + folder + "` — already running");
            return false;
        }
    
        const std::string env = env_name(folder);
        const auto* res = get_resource(folder);
        vm -> create_environment();
        vm -> set_reference(env);
        bool ok = true;
    
        for (const auto& script : res->scripts) {
            if (!is_eligible(script.type)) continue;
            std::string source;
            try {
                source = Vital::Tool::File::read_text(resource_path(folder), script.src);
            }
            catch (...) {
                Vital::print("error", "Resource `" + folder + "` failed to read script `" + script.src + "`");
                ok = false;
                break;
            }
            vm -> get_reference(env, true);
            if (!vm -> load_string(source, true, true, vm -> get_count())) {
                Vital::print("error", "Resource `" + folder + "` failed to execute script `" + script.src + "`");
                vm -> pop();
                ok = false;
                break;
            }
            vm -> pop();
        }

        if (!ok) {
            vm -> del_reference(env);
            return false;
        }

        running.insert(folder);
        Vital::print("sbox", "Resource `" + folder + "` started");
        Sandbox::get_singleton() -> signal("vital.resource:started", Vital::Tool::StackValue(folder));
        return true;
    }

    bool ResourceManager::stop(const std::string& folder) {
        auto* vm = Sandbox::get_singleton() -> get_vm();
        if (!is_running(folder)) {
            Vital::print("error", "Cannot stop `" + folder + "` — not running");
            return false;
        }
    
        Sandbox::get_singleton() -> signal("vital.resource:stopped", Vital::Tool::StackValue(folder));
        vm -> del_reference(env_name(folder));
        running.erase(folder);
        Vital::print("sbox", "Resource `" + folder + "` stopped");
        return true;
    }

    bool ResourceManager::restart(const std::string& folder) {
        if (is_running(folder)) stop(folder);
        return start(folder);
    }

    void ResourceManager::start_all() {
        for (const auto* res : get_all_resources()) {
            start(res->folder);
        }
    }

    void ResourceManager::stop_all() {
        std::unordered_set<std::string> snapshot = running;
        for (const auto& folder : snapshot) {
            stop(folder);
        }
    }
}