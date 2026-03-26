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


    // Checkers //
    bool ResourceManager::is_eligible(const std::string& type) {
        if (type == "shared") return true;
        return type == Vital::get_platform();
    }

    bool ResourceManager::is_loaded(const std::string& name) const {
        for (const auto& resource : resources) {
            if (resource.ref == name) {
                return true;
            }
        }
        return false;
    }

    bool ResourceManager::is_running(const std::string& name) const {
        return running.count(name) > 0;
    }


    // Getters //
    std::string ResourceManager::get_resource_base(const std::string& name) {
        return Vital::get_directory("resources", name);
    }

    std::string ResourceManager::get_resource_env(const std::string& name) {
        return name + ":env";
    }

    std::vector<const ResourceManager::ResourceManifest*> ResourceManager::get_all_resources() const {
        std::vector<const ResourceManager::ResourceManifest*> result;
        result.reserve(resources.size());
        for (const auto& resource : resources) {
            result.push_back(&resource);
        }
        return result;
    }

    const ResourceManager::ResourceManifest* ResourceManager::get_resource(const std::string& name) const {
        for (const auto& resource : resources) {
            if (resource.ref == name) {
                return &resource;
            }
        }
        return nullptr;
    }

    std::vector<ResourceManager::ResourceScript> ResourceManager::get_scripts(const std::string& name, const std::string& type) const {
        std::vector<ResourceManager::ResourceScript> result;
        const auto* resource = get_resource(name);
        if (!resource) return result;
        for (const auto& script : resource -> scripts) {
            if (type.empty() || script.type == type) {
                result.push_back(script);
            }
        }
        return result;
    }


    // APIs //
    void ResourceManager::scan() {
        Vital::print("sbox", "Rescanning resources...");
        resources.clear();
        std::vector<std::string> folders;

        try {
            folders = Vital::Tool::File::contents(Vital::get_directory(), "resources", true);
        }
        catch (...) {
            Vital::print("error", "Resource scan skipped — `resources/` directory not found");
            return;
        }

        std::unordered_map<std::string, int> folder_count;
        for (const auto& folder_path : folders) {
            const std::string name = folder_path.substr(folder_path.find_last_of("/\\") + 1);
            folder_count[name]++;
        }

        for (const auto& folder_path : folders) {
            const std::string name = folder_path.substr(folder_path.find_last_of("/\\") + 1);
            if (folder_count[name] > 1) {
                Vital::print("error", "Duplicate resource found — skipping `" + name + "`");
                continue;
            }
            if (!Vital::Tool::File::exists(get_resource_base(name), "manifest.yaml")) continue;

            YAML::Node manifest;
            try {
                const std::string content = Vital::Tool::File::read_text(get_resource_base(name), "manifest.yaml");
                manifest = YAML::Load(content);
            }
            catch (const YAML::Exception& e) {
                Vital::print("error", "Failed to parse manifest for `" + name + "` — " + e.what());
                continue;
            }
            catch (...) {
                Vital::print("error", "Failed to read manifest for `" + name + "`");
                continue;
            }

            ResourceManifest resource;
            resource.ref = name;
            resource.name = manifest["name"] ? manifest["name"].as<std::string>() : name;
            resource.author = manifest["author"] ? manifest["author"].as<std::string>() : "";
            resource.version = manifest["version"] ? manifest["version"].as<std::string>() : "";
            if (!manifest["scripts"] || !manifest["scripts"].IsSequence()) {
                Vital::print("error", "Resource `" + name + "` has no valid `scripts` section — skipping");
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
                if (!Vital::Tool::File::exists(get_resource_base(name), src)) {
                    errors.push_back("script `" + src + "` not found on disk");
                    valid = false;
                    continue;
                }
                resource.scripts.push_back({ src, type });
            }

            if (manifest["files"] && manifest["files"].IsSequence()) {
                for (const auto& node : manifest["files"]) {
                    const std::string file = node.as<std::string>();
                    if (!Vital::Tool::File::exists(get_resource_base(name), file)) {
                        errors.push_back("file `" + file + "` not found on disk");
                        valid = false;
                        continue;
                    }
                    resource.files.push_back(file);
                }
            }

            if (!valid) {
                std::string error_list = "Resource `" + name + "` skipped — " + std::to_string(errors.size()) + " error(s):\n";
                for (const auto& err : errors) {
                    error_list += "> " + err + "\n";
                }
                Vital::print("error", error_list);
                continue;
            }
            resources.push_back(std::move(resource));
            Vital::print("sbox", "Loaded resource `" + name + "`");
        }

        Vital::print("sbox", "Resource scan complete — " + std::to_string(resources.size()) + " resource(s) loaded");
    }

    bool ResourceManager::start(const std::string& name) {
        auto* vm = Sandbox::get_singleton() -> get_vm();
        if (!is_loaded(name)) {
            Vital::print("error", "Cannot start `" + name + "` — resource not loaded");
            return false;
        }
        if (is_running(name)) {
            Vital::print("error", "Cannot start `" + name + "` — already running");
            return false;
        }
    
        const std::string env = get_resource_env(name);
        const auto* resource = get_resource(name);
        bool status = true;
        vm -> create_environment();
        vm -> set_reference(env);

        for (const auto& script : resource -> scripts) {
            if (!is_eligible(script.type)) continue;
            std::string source;
            try {
                source = Vital::Tool::File::read_text(get_resource_base(name), script.src);
            }
            catch (...) {
                Vital::print("error", "Resource `" + name + "` failed to read script `" + script.src + "`");
                status = false;
                break;
            }
            vm -> get_reference(env, true);
            if (!vm -> load_string(source, true, true, vm -> get_count())) {
                Vital::print("error", "Resource `" + name + "` failed to execute script `" + script.src + "`");
                vm -> pop();
                status = false;
                break;
            }
            vm -> pop();
        }
        if (!status) {
            vm -> del_reference(env);
            return false;
        }

        running.insert(name);
        Vital::print("sbox", "Resource `" + name + "` started");
        Sandbox::get_singleton() -> signal("vital.resource:started", Vital::Tool::StackValue(name));
        return true;
    }

    bool ResourceManager::stop(const std::string& name) {
        auto* vm = Sandbox::get_singleton() -> get_vm();
        if (!is_running(name)) {
            Vital::print("error", "Cannot stop `" + name + "` — not running");
            return false;
        }
    
        Sandbox::get_singleton() -> signal("vital.resource:stopped", Vital::Tool::StackValue(name));
        vm -> del_reference(get_resource_env(name));
        running.erase(name);
        Vital::print("sbox", "Resource `" + name + "` stopped");
        return true;
    }

    bool ResourceManager::restart(const std::string& name) {
        if (is_running(name)) stop(name);
        return start(name);
    }

    void ResourceManager::start_all() {
        for (const auto* resource : get_all_resources()) {
            start(resource -> name);
        }
    }

    void ResourceManager::stop_all() {
        std::unordered_set<std::string> snapshot = running;
        for (const auto& name : snapshot) {
            stop(name);
        }
    }
}