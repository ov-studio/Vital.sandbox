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
#include <Vital.extension/Engine/public/asset.h>
#include <Vital.extension/Engine/public/network.h>
#include <Vital.extension/Sandbox/index.h>
#include <yaml-cpp/yaml.h>


//////////////////////////////
// Vital: Engine: Resource //
//////////////////////////////

namespace Vital::Engine {
    // TODO: Improve
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
            if (resource.ref == name) return true;
        }
        return false;
    }

    bool ResourceManager::is_running(const std::string& name) const {
        return running.count(name) > 0;
    }

    #if defined(Vital_SDK_Client)
    bool ResourceManager::is_pending(const std::string& name) const {
        return pending.count(name) > 0;
    }
    #endif


    // Getters //
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
            if (resource.ref == name) return &resource;
        }
        return nullptr;
    }

    std::string ResourceManager::get_resource_base(const std::string& name) {
        return Vital::get_directory("resources", name);
    }

    std::string ResourceManager::get_resource_env(const std::string& name) {
        return name + ":env";
    }

    std::vector<ResourceManager::ResourceScript> ResourceManager::get_resource_scripts(const std::string& name, const std::string& type) const {
        std::vector<ResourceManager::ResourceScript> result;
        const auto* resource = get_resource(name);
        if (!resource) return result;
        for (const auto& script : resource->scripts) {
            if (type.empty() || script.type == type) result.push_back(script);
        }
        return result;
    }


    // APIs //
    void ResourceManager::scan() {
        Vital::print("sbox", "Rescanning resources...");
        resources.clear();

        std::vector<std::string> contents;
        try {
            contents = Vital::Tool::File::contents(Vital::get_directory(), "resources", true);
        }
        catch (...) {
            Vital::print("error", "Resource scan skipped — `resources/` directory not found");
            return;
        }

        // First pass — detect duplicates
        std::unordered_map<std::string, int> resource_count;
        for (const auto& path : contents) {
            const std::string name = path.substr(path.find_last_of("/\\") + 1);
            resource_count[name]++;
        }

        // Second pass — load valid resources
        for (const auto& path : contents) {
            const std::string name = path.substr(path.find_last_of("/\\") + 1);

            if (resource_count[name] > 1) {
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
            resource.ref     = name;
            resource.name    = manifest["name"]    ? manifest["name"].as<std::string>()    : name;
            resource.author  = manifest["author"]  ? manifest["author"].as<std::string>()  : "";
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
                if (valid_types.find(type) == valid_types.end()) {
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
                for (const auto& err : errors) error_list += "> " + err + "\n";
                Vital::print("error", error_list);
                continue;
            }

            resources.push_back(std::move(resource));
            Vital::print("sbox", "Loaded resource `" + name + "`");
        }

        Vital::print("sbox", "Resource scan complete — " + std::to_string(resources.size()) + " resource(s) loaded");

        // Wire up network-driven lifecycle events once per scan
        #if !defined(Vital_SDK_Client)
        static bool server_bound = false;
        if (!server_bound) {
            server_bound = true;
            Vital::Tool::Event::bind("vital.network:peer_connected", [](Vital::Tool::Stack args) -> void {
                if (args.array.empty()) return;
                const int peer_id = args.array[0].as<int32_t>();
                // Deferred so Godot multiplayer is accessed on the main thread
                Core::get_singleton()->call_deferred(
                    "broadcast_asset_manifest",
                    peer_id
                );
            });
        }
        #endif

        #if defined(Vital_SDK_Client)
        static bool client_bound = false;
        if (!client_bound) {
            client_bound = true;
            Vital::Tool::Event::bind("vital.resource:started", [](Vital::Tool::Stack args) -> void {
                if (args.array.empty()) return;
                const std::string name = args.array[0].as<std::string>();
                ResourceManager::get_singleton()->load(name);
            });
            Vital::Tool::Event::bind("vital.resource:stopped", [](Vital::Tool::Stack args) -> void {
                if (args.array.empty()) return;
                const std::string name = args.array[0].as<std::string>();
                ResourceManager::get_singleton()->unload(name);
            });
        }
        #endif
    }


    #if !defined(Vital_SDK_Client)

    bool ResourceManager::start(const std::string& name) {
        auto* vm = Sandbox::get_singleton()->get_vm();
        auto* am = AssetManager::get_singleton();

        if (!is_loaded(name)) {
            Vital::print("error", "Cannot start `" + name + "` — resource not loaded");
            return false;
        }
        if (is_running(name)) {
            Vital::print("error", "Cannot start `" + name + "` — already running");
            return false;
        }

        const std::string env = get_resource_env(name);
        const auto* resource  = get_resource(name);
        bool status           = true;

        vm->create_environment();
        vm->set_reference(env);

        // Execute server + shared scripts locally
        for (const auto& script : resource->scripts) {
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

            vm->get_reference(env, true);
            if (!vm->load_string(source, true, true, vm->get_count())) {
                Vital::print("error", "Resource `" + name + "` failed to execute script `" + script.src + "`");
                vm->pop();
                status = false;
                break;
            }
            vm->pop();
        }

        if (!status) {
            vm->del_reference(env);
            return false;
        }

        // Register files + client/shared scripts as relative paths
        for (const auto& file : resource->files) {
            am->register_asset("resources/" + name + "/" + file);
        }
        for (const auto& script : resource->scripts) {
            if (script.type == "client" || script.type == "shared") {
                am->register_asset("resources/" + name + "/" + script.src);
            }
        }

        // Broadcast updated manifest to all currently connected clients
        // deferred to ensure Godot multiplayer runs on the main thread
        am->broadcast_manifest_deferred();

        running.insert(name);
        Vital::print("sbox", "Resource `" + name + "` started");
        Sandbox::get_singleton()->signal("vital.resource:started", Vital::Tool::StackValue(name));
        return true;
    }

    bool ResourceManager::stop(const std::string& name) {
        auto* vm = Sandbox::get_singleton()->get_vm();

        if (!is_running(name)) {
            Vital::print("error", "Cannot stop `" + name + "` — not running");
            return false;
        }

        // Signal reaches clients — they immediately cancel downloads and unload
        Sandbox::get_singleton()->signal("vital.resource:stopped", Vital::Tool::StackValue(name));

        vm->del_reference(get_resource_env(name));
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
            start(resource->ref);
        }
    }

    void ResourceManager::stop_all() {
        std::unordered_set<std::string> snapshot = running;
        for (const auto& name : snapshot) {
            stop(name);
        }
    }

    #endif


    #if defined(Vital_SDK_Client)

    bool ResourceManager::load(const std::string& name) {
        auto* am = AssetManager::get_singleton();

        if (!is_loaded(name)) {
            Vital::print("error", "Cannot load `" + name + "` — resource not scanned");
            return false;
        }
        if (is_running(name) || is_pending(name)) {
            Vital::print("error", "Cannot load `" + name + "` — already running or pending");
            return false;
        }

        const auto* resource = get_resource(name);

        // Collect relative asset paths matching what server registered
        std::unordered_set<std::string> asset_paths;
        for (const auto& file : resource->files) {
            asset_paths.insert("resources/" + name + "/" + file);
        }
        for (const auto& script : resource->scripts) {
            if (script.type == "client" || script.type == "shared") {
                asset_paths.insert("resources/" + name + "/" + script.src);
            }
        }

        resource_assets[name] = asset_paths;
        pending.insert(name);

        Vital::print("sbox", "Resource `" + name + "` queued for download — " + std::to_string(asset_paths.size()) + " asset(s)");

        // Once all assets are downloaded, execute scripts into isolated env
        Vital::Tool::Event::bind("asset:ready", [name](Vital::Tool::Stack) -> void {
            auto* rm = ResourceManager::get_singleton();
            if (!rm->is_pending(name)) return;

            auto* vm              = Sandbox::get_singleton()->get_vm();
            auto* am              = AssetManager::get_singleton();
            const auto* resource  = rm->get_resource(name);
            const std::string env = get_resource_env(name);
            bool status           = true;

            vm->create_environment();
            vm->set_reference(env);

            for (const auto& script : resource->scripts) {
                if (script.type != "shared" && script.type != "client") continue;

                // Read from vital_assets/ using the flattened filename
                const std::string asset_path = "resources/" + name + "/" + script.src;
                std::string source;
                try {
                    source = Vital::Tool::File::read_text(am->get_local_base(), asset_path);
                }
                catch (...) {
                    Vital::print("error", "Resource `" + name + "` failed to read script `" + script.src + "`");
                    status = false;
                    break;
                }

                vm->get_reference(env, true);
                if (!vm->load_string(source, true, true, vm->get_count())) {
                    Vital::print("error", "Resource `" + name + "` failed to execute script `" + script.src + "`");
                    vm->pop();
                    status = false;
                    break;
                }
                vm->pop();
            }

            rm->pending.erase(name);
            rm->resource_assets.erase(name);

            if (!status) {
                vm->del_reference(env);
                Vital::print("error", "Resource `" + name + "` failed to load on client — env released");
                return;
            }

            rm->running.insert(name);
            Vital::print("sbox", "Resource `" + name + "` loaded on client");
            Sandbox::get_singleton()->signal("vital.resource:started", Vital::Tool::StackValue(name));
        });

        return true;
    }

    bool ResourceManager::unload(const std::string& name) {
        auto* vm = Sandbox::get_singleton()->get_vm();
        auto* am = AssetManager::get_singleton();

        if (!is_running(name) && !is_pending(name)) {
            Vital::print("error", "Cannot unload `" + name + "` — not running or pending");
            return false;
        }

        // Cancel all in-progress downloads for this resource immediately
        if (is_pending(name)) {
            auto it = resource_assets.find(name);
            if (it != resource_assets.end()) {
                for (const auto& path : it->second) {
                    am->cancel(path);
                }
                resource_assets.erase(it);
            }
            pending.erase(name);
            Vital::print("sbox", "Resource `" + name + "` download cancelled");
        }

        // Release env if scripts were already executed
        if (is_running(name)) {
            vm->del_reference(get_resource_env(name));
            running.erase(name);
        }

        Vital::print("sbox", "Resource `" + name + "` unloaded on client");
        Sandbox::get_singleton()->signal("vital.resource:stopped", Vital::Tool::StackValue(name));
        return true;
    }

    #endif
}