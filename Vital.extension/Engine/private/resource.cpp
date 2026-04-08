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
#include <Vital.sandbox/Tool/yaml.h>


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
    bool ResourceManager::is_name(const std::string& name) {
        if (name.empty() || !Vital::Tool::File::sanitize(name)) return false;
        for (const char c : name) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && (c != '_'))
                return false;
        }
        return true;
    }

    bool ResourceManager::is_type(const std::string& type) {
        if (type == "shared") return true;
        return type == Vital::get_platform();
    }

    bool ResourceManager::is_loaded(const std::string& name) const {
        for (const auto& resource : resources) {
            if (resource.ref == name)
                return true;
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
        for (const auto& resource : resources)
            result.push_back(&resource);
        return result;
    }

    const ResourceManager::ResourceManifest* ResourceManager::get_resource(const std::string& name) const {
        for (const auto& resource : resources)
            if (resource.ref == name) return &resource;
        return nullptr;
    }

    std::string ResourceManager::get_resource_from_vm(Vital::Sandbox::Machine* vm) {
        return vm->get_environment_id();
    }

    std::string ResourceManager::get_resource_base(const std::string& name, bool require_running) {
        if (!is_name(name)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error, "invalid resource name");
        if (require_running && !get_singleton() -> is_running(name)) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error, "resource not running");
        return Vital::get_directory("resources", name);
    }

    std::vector<ResourceManager::ResourceScript> ResourceManager::get_resource_scripts(const std::string& name, const std::string& type) const {
        std::vector<ResourceManager::ResourceScript> result;
        const auto* resource = get_resource(name);
        if (!resource) return result;
        for (const auto& script : resource->scripts) {
            if (type.empty() || script.type == type)
                result.push_back(script);
        }
        return result;
    }


    // APIs //
    #if !defined(Vital_SDK_Client)
    bool ResourceManager::parse_manifest(ResourceManifest& resource, Vital::Tool::YAML& manifest, const std::string& base, std::vector<std::string>& errors) {
        resource.name    = manifest.get_str("name",    resource.ref);
        resource.author  = manifest.get_str("author",  "");
        resource.version = manifest.get_str("version", "");
        resource.scripts.clear();
        resource.files.clear();
        resource.script_hashes.clear();
        resource.file_hashes.clear();

        if (!manifest.has("scripts") || !manifest.get_root()["scripts"].is_seq())
            return false;

        for (ryml::ConstNodeRef node : manifest.get_root()["scripts"]) {
            if (!node.is_map() || !Vital::Tool::YAML::has(node, "src") || !Vital::Tool::YAML::has(node, "type")) {
                errors.push_back("script entry missing `src` or `type`");
                continue;
            }
            const std::string src  = Vital::Tool::YAML::get_str(node, "src");
            const std::string type = Vital::Tool::YAML::get_str(node, "type");
            if (valid_types.find(type) == valid_types.end()) {
                errors.push_back(fmt::format("script `{}` has invalid type `{}`", src, type));
                continue;
            }
            std::vector<std::string> expanded = Vital::Tool::File::glob(base, src);
            if (expanded.empty()) {
                errors.push_back(fmt::format("script pattern `{}` matched no files", src));
                continue;
            }
            for (const auto& s : expanded) {
                resource.scripts.push_back({ s, type });
                resource.script_hashes[s] = Vital::Tool::File::hash(base, s);
            }
        }

        if (manifest.has("files") && manifest.get_root()["files"].is_seq()) {
            for (ryml::ConstNodeRef node : manifest.get_root()["files"]) {
                std::string file_pattern;
                node >> file_pattern;
                std::vector<std::string> expanded = Vital::Tool::File::glob(base, file_pattern);
                if (expanded.empty()) {
                    errors.push_back(fmt::format("file pattern `{}` matched no files", file_pattern));
                    continue;
                }
                for (const auto& f : expanded) {
                    resource.files.push_back(f);
                    resource.file_hashes[f] = Vital::Tool::File::hash(base, f);
                }
            }
        }

        return errors.empty();
    }

    #endif

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

        std::unordered_map<std::string, int> resource_count;
        for (const auto& path : contents) {
            const std::string name = path.substr(path.find_last_of("/\\") + 1);
            resource_count[name]++;
        }

        for (const auto& path : contents) {
            const std::string name = path.substr(path.find_last_of("/\\") + 1);
            if (!is_name(name)) {
                Vital::print("error", fmt::format("Invalid resource name `{}` — skipping", name));
                continue;
            }
            if (resource_count[name] > 1) {
                Vital::print("error", fmt::format("Duplicate resource found — skipping `{}`", name));
                continue;
            }
            if (!Vital::Tool::File::exists(get_resource_base(name), "manifest.yaml")) continue;

            std::string content;
            try {
                content = Vital::Tool::File::read_text(get_resource_base(name), "manifest.yaml");
            }
            catch (...) {
                Vital::print("error", fmt::format("Failed to read manifest for `{}` — skipping", name));
                continue;
            }

            Vital::Tool::YAML manifest;
            try {
                manifest.parse(content);
            }
            catch (const std::exception& e) {
                Vital::print("error", fmt::format(
                    "Malformed YAML in manifest for `{}` — skipping\n"
                    "> {}",
                    name, e.what()
                ));
                continue;
            }

            ResourceManifest resource;
            resource.ref = name;

            std::vector<std::string> errors;
            #if !defined(Vital_SDK_Client)
            if (!parse_manifest(resource, manifest, get_resource_base(name), errors)) {
            #else
            resource.name    = manifest.get_str("name",    name);
            resource.author  = manifest.get_str("author",  "");
            resource.version = manifest.get_str("version", "");
            if (false) {
            #endif
                if (!errors.empty()) {
                    std::string error_list = fmt::format("Resource `{}` skipped — {} error(s):\n", name, errors.size());
                    for (const auto& err : errors) error_list += fmt::format("> {}\n", err);
                    Vital::print("error", error_list);
                }
                else {
                    Vital::print("error", fmt::format("Resource `{}` has no valid `scripts` section — skipping", name));
                }
                continue;
            }

            resources.push_back(std::move(resource));
            Vital::print("sbox", fmt::format("Loaded resource `{}`", name));
        }

        Vital::print("sbox", fmt::format("Resource scan complete — {} resource(s) loaded", resources.size()));

        #if !defined(Vital_SDK_Client)
        {
            std::vector<std::string> stale;
            for (const auto& name : running) {
                if (!is_loaded(name)) stale.push_back(name);
            }
            for (const auto& name : stale) {
                Vital::print("sbox", fmt::format("Resource `{}` no longer exists — stopping", name));
                stop(name);
            }
        }

        static bool server_bound = false;
        if (!server_bound) {
            server_bound = true;
            Vital::Tool::Event::bind("vital.network:peer_connected", [](Vital::Tool::Stack args) {
                if (args.array.empty()) return;
                const int peer_id = args.array[0].as<int32_t>();
                Core::get_singleton() -> push_deferred([peer_id]() {
                    AssetManager::get_singleton() -> broadcast_manifest(peer_id);
                });
            });
        }
        #endif
    }

    void ResourceManager::init() {
        #if defined(Vital_SDK_Client)
        static bool client_initialized = false;
        if (client_initialized) return;
        client_initialized = true;

        Vital::print("sbox", "Initializing client resource manager...");

        Vital::Tool::Event::bind("asset:file_ready", [](Vital::Tool::Stack args) {
            if (!args.object.count("path")) return;
            const std::string path = args.object.at("path").as<std::string>();
            auto* rm = ResourceManager::get_singleton();
            for (auto& [name, remaining] : rm->resource_assets) {
                if (!remaining.count(path)) continue;
                remaining.erase(path);
                Vital::print("sbox", fmt::format("Resource `{}` asset ready: {}", name, path));
                if (remaining.empty()) rm->execute_scripts(name);
                break;
            }
        });

        Vital::Tool::Event::bind("network:packet", [](Vital::Tool::Stack args) {
            if (!args.object.count("type")) return;
            const std::string type = args.object.at("type").as<std::string>();
            auto* rm = ResourceManager::get_singleton();

            if (type == "vital.resource:started") {
                if (!args.object.count("name")) return;
                const std::string name = args.object.at("name").as<std::string>();

                std::vector<ResourceManager::ResourceScript> scripts;
                if (args.object.count("script_count")) {
                    const int count = args.object.at("script_count").as<int32_t>();
                    scripts.reserve(count);
                    for (int i = 0; i < count; i++) {
                        const std::string src_key  = fmt::format("script_src_{}", i);
                        const std::string type_key = fmt::format("script_type_{}", i);
                        if (!args.object.count(src_key) || !args.object.count(type_key)) continue;
                        scripts.push_back({
                            args.object.at(src_key).as<std::string>(),
                            args.object.at(type_key).as<std::string>()
                        });
                    }
                }

                std::vector<std::string> files;
                if (args.object.count("file_count")) {
                    const int count = args.object.at("file_count").as<int32_t>();
                    files.reserve(count);
                    for (int i = 0; i < count; i++) {
                        const std::string key = fmt::format("file_{}", i);
                        if (!args.object.count(key)) continue;
                        files.push_back(args.object.at(key).as<std::string>());
                    }
                }

                rm->register_remote(name, scripts, files);
                Vital::print("sbox", fmt::format("Client received resource start: `{}`", name));
                if (!rm->is_running(name) && !rm->is_pending(name)) rm->load(name);
            }
            else if (type == "vital.resource:stopped") {
                if (!args.object.count("name")) return;
                const std::string name = args.object.at("name").as<std::string>();
                Vital::print("sbox", fmt::format("Client received resource stop: `{}`", name));
                rm->unload(name);
            }
        });

        Vital::print("sbox", "Client resource manager initialized");
        #endif
    }


    #if !defined(Vital_SDK_Client)

    void ResourceManager::notify_resource_started(const std::string& name) {
        const auto* resource = get_resource(name);
        if (!resource) return;

        Vital::Tool::Stack msg;
        msg.object["type"] = Vital::Tool::StackValue(std::string("vital.resource:started"));
        msg.object["name"] = Vital::Tool::StackValue(name);

        msg.object["script_count"] = Vital::Tool::StackValue((int32_t)resource->scripts.size());
        for (int i = 0; i < (int)resource->scripts.size(); i++) {
            msg.object[fmt::format("script_src_{}", i)]  = Vital::Tool::StackValue(resource->scripts[i].src);
            msg.object[fmt::format("script_type_{}", i)] = Vital::Tool::StackValue(resource->scripts[i].type);
        }

        msg.object["file_count"] = Vital::Tool::StackValue((int32_t)resource->files.size());
        for (int i = 0; i < (int)resource->files.size(); i++) {
            msg.object[fmt::format("file_{}", i)] = Vital::Tool::StackValue(resource->files[i]);
        }

        for (int peer_id : Vital::Engine::Network::get_singleton() -> get_connected_peers()) {
            Vital::Engine::Network::get_singleton() -> send(msg, peer_id);
        }
    }

    void ResourceManager::notify_resource_stopped(const std::string& name) {
        Vital::Tool::Stack msg;
        msg.object["type"] = Vital::Tool::StackValue(std::string("vital.resource:stopped"));
        msg.object["name"] = Vital::Tool::StackValue(name);
        for (int peer_id : Vital::Engine::Network::get_singleton() -> get_connected_peers()) {
            Vital::Engine::Network::get_singleton() -> send(msg, peer_id);
        }
    }

    bool ResourceManager::start(const std::string& name) {
        auto* vm = Sandbox::get_singleton() -> get_vm();
        auto* am = AssetManager::get_singleton();

        if (!is_loaded(name)) {
            Vital::print("error", fmt::format("Cannot start `{}` — resource not loaded", name));
            return false;
        }
        if (is_running(name)) {
            Vital::print("error", fmt::format("Cannot start `{}` — already running", name));
            return false;
        }

        const auto* resource = get_resource(name);

        // Phase 1: Register assets grouped under resource name
        std::vector<std::string> asset_paths;
        for (const auto& file : resource->files)
            asset_paths.push_back(fmt::format("resources/{}/{}", name, file));
        for (const auto& script : resource->scripts) {
            if (script.type == "client" || script.type == "shared")
                asset_paths.push_back(fmt::format("resources/{}/{}", name, script.src));
        }
        am->register_assets(asset_paths, name);
        am->broadcast_manifest_deferred();

        // Phase 2: Mark as started and notify
        running.insert(name);
        Vital::print("sbox", fmt::format("Resource `{}` started", name));
        Core::get_singleton() -> push_deferred([this, name]() {
            notify_resource_started(name);
        });
        Sandbox::get_singleton() -> signal("vital.resource:started", Vital::Tool::StackValue(name));

        // Phase 3: Execute scripts
        vm->create_environment(name);
        vm->pop();

        for (const auto& script : resource->scripts) {
            if (!is_type(script.type)) continue;

            std::string source;
            try {
                source = Vital::Tool::File::read_text(get_resource_base(name), script.src);
            }
            catch (...) {
                Vital::print("error", fmt::format("Resource `{}` failed to read script `{}`", name, script.src));
                running.erase(name);
                vm->clear_environment_id(name);
                am->unregister_group(name);
                return false;
            }

            vm->get_reference(name, true);
            if (!vm->load_string(source, true, true, vm->get_count())) {
                Vital::print("error", fmt::format("Resource `{}` failed to execute script `{}`", name, script.src));
                vm->pop();
                running.erase(name);
                vm->clear_environment_id(name);
                am->unregister_group(name);
                return false;
            }
            vm->pop();
        }

        return true;
    }

    bool ResourceManager::stop(const std::string& name) {
        auto* vm = Sandbox::get_singleton() -> get_vm();
        auto* am = AssetManager::get_singleton();
        
        if (!is_running(name)) {
            Vital::print("error", fmt::format("Cannot stop `{}` — not running", name));
            return false;
        }

        am->unregister_group(name);
        Core::get_singleton() -> push_deferred([this, name]() {
            notify_resource_stopped(name);
        });

        Sandbox::get_singleton() -> signal("vital.resource:stopped", Vital::Tool::StackValue(name));
        vm->clear_environment_id(name);

        running.erase(name);
        Vital::print("sbox", fmt::format("Resource `{}` stopped", name));
        return true;
    }

    bool ResourceManager::restart(const std::string& name) {
        if (!is_running(name)) {
            Vital::print("error", fmt::format("Cannot restart `{}` — not running", name));
            return false;
        }

        stop(name);
        const std::string base = get_resource_base(name);
        if (!Vital::Tool::File::exists(base, "manifest.yaml")) {
            resources.erase(
                std::remove_if(resources.begin(), resources.end(),
                    [&name](const ResourceManifest& m) { return m.ref == name; }),
                resources.end()
            );
            Vital::print("error", fmt::format("Resource `{}` manifest not found — unregistered", name));
            return false;
        }

        std::string content;
        try {
            content = Vital::Tool::File::read_text(base, "manifest.yaml");
        }
        catch (...) {
            Vital::print("error", fmt::format("Resource `{}` failed to read manifest — skipping restart", name));
            return false;
        }

        Vital::Tool::YAML manifest;
        try {
            manifest.parse(content);
        }
        catch (const std::exception& e) {
            Vital::print("error", fmt::format("Resource `{}` malformed YAML — {} — skipping restart", name, e.what()));
            return false;
        }

        for (auto& resource : resources) {
            if (resource.ref != name) continue;

            // Snapshot old hashes
            const auto old_script_hashes = resource.script_hashes;
            const auto old_file_hashes   = resource.file_hashes;

            std::vector<std::string> errors;
            if (!parse_manifest(resource, manifest, base, errors)) {
                if (!errors.empty()) {
                    std::string error_list = fmt::format("Resource `{}` restart aborted — {} error(s):\n", name, errors.size());
                    for (const auto& err : errors) error_list += fmt::format("> {}\n", err);
                    Vital::print("error", error_list);
                }
                else {
                    Vital::print("error", fmt::format("Resource `{}` has no valid `scripts` section — skipping restart", name));
                }
                return false;
            }

            // Diff scripts
            std::vector<std::string> changes;
            for (const auto& [src, hash] : old_script_hashes) {
                if (!resource.script_hashes.count(src))
                    changes.push_back(fmt::format("> `{}` (script deleted)", src));
                else if (resource.script_hashes.at(src) != hash)
                    changes.push_back(fmt::format("> `{}` (script modified)", src));
            }
            for (const auto& [src, hash] : resource.script_hashes) {
                if (!old_script_hashes.count(src))
                    changes.push_back(fmt::format("> `{}` (script added)", src));
            }

            // Diff files
            for (const auto& [file, hash] : old_file_hashes) {
                if (!resource.file_hashes.count(file))
                    changes.push_back(fmt::format("> `{}` (file deleted)", file));
                else if (resource.file_hashes.at(file) != hash)
                    changes.push_back(fmt::format("> `{}` (file modified)", file));
            }
            for (const auto& [file, hash] : resource.file_hashes) {
                if (!old_file_hashes.count(file))
                    changes.push_back(fmt::format("> `{}` (file added)", file));
            }

            // Report
            std::string report = fmt::format("Resource `{}` restarted\n", name);
            if (changes.empty()) {
                report += "> No changes detected";
            }
            else {
                report += fmt::format("> Changes ({}):\n", changes.size());
                for (const auto& change : changes)
                    report += change + "\n";
            }
            Vital::print("sbox", report);

            break;
        }

        return start(name);
    }

    void ResourceManager::start_all() {
        Vital::print("sbox", "Starting all resources...");
        int count = 0;
        for (const auto* resource : get_all_resources())
            if (start(resource->ref)) count++;
        Vital::print("sbox", fmt::format("All resources started — {} resource(s) running", count));
    }
    
    void ResourceManager::stop_all() {
        Vital::print("sbox", "Stopping all resources...");
        std::unordered_set<std::string> snapshot = running;
        int count = 0;
        for (const auto& name : snapshot)
            if (stop(name)) count++;
        Vital::print("sbox", fmt::format("All resources stopped — {} resource(s) stopped", count));
    }
    
    void ResourceManager::restart_all() {
        Vital::print("sbox", "Restarting all resources...");
        std::unordered_set<std::string> snapshot = running;
        int count = 0;
        for (const auto& name : snapshot)
            if (restart(name)) count++;
        Vital::print("sbox", fmt::format("All resources restarted — {} resource(s) restarted", count));
    }
    #endif


    #if defined(Vital_SDK_Client)

    bool ResourceManager::register_remote(const std::string& name, const std::vector<ResourceScript>& scripts, const std::vector<std::string>& files) {
        unregister_remote(name);
        ResourceManifest manifest;
        manifest.ref     = name;
        manifest.name    = name;
        manifest.scripts = scripts;
        manifest.files   = files;
        resources.push_back(std::move(manifest));
        Vital::print("sbox", fmt::format("Resource `{}` manifest registered from server — {} script(s), {} file(s)", name, scripts.size(), files.size()));
        return true;
    }

    void ResourceManager::unregister_remote(const std::string& name) {
        resources.erase(
            std::remove_if(resources.begin(), resources.end(),
                [&name](const ResourceManifest& m) { return m.ref == name; }),
            resources.end()
        );
    }

    bool ResourceManager::load(const std::string& name) {
        auto* am = AssetManager::get_singleton();

        if (!is_loaded(name)) {
            Vital::print("error", fmt::format("Cannot load `{}` — resource not registered", name));
            return false;
        }
        if (is_running(name) || is_pending(name)) {
            Vital::print("error", fmt::format("Cannot load `{}` — already running or pending", name));
            return false;
        }

        const auto* resource = get_resource(name);
        if (!resource) {
            Vital::print("error", fmt::format("Cannot load `{}` — manifest is null", name));
            return false;
        }

        std::unordered_set<std::string> asset_paths;
        for (const auto& file : resource->files)
            asset_paths.insert(fmt::format("resources/{}/{}", name, file));
        for (const auto& script : resource->scripts) {
            if (script.type == "client" || script.type == "shared")
                asset_paths.insert(fmt::format("resources/{}/{}", name, script.src));
        }

        pending.insert(name);

        for (const auto& path : asset_paths) {
            if (Vital::Tool::File::exists(get_directory(), path)) {
                Vital::print("sbox", fmt::format("Resource `{}` asset cached: {}", name, path));
            } else {
                resource_assets[name].insert(path);
            }
        }

        Vital::print("sbox", fmt::format("Resource `{}` queued — {} asset(s) pending download", name, resource_assets[name].size()));

        if (resource_assets[name].empty()) {
            Vital::print("sbox", fmt::format("Resource `{}` all assets cached — executing immediately", name));
            execute_scripts(name);
        }

        return true;
    }

    void ResourceManager::execute_scripts(const std::string& name) {
        if (!is_pending(name)) return;

        auto* vm             = Sandbox::get_singleton() -> get_vm();
        auto* am             = AssetManager::get_singleton();
        const auto* resource = get_resource(name);

        if (!resource) {
            Vital::print("error", fmt::format("execute_scripts: manifest null for `{}` — aborting", name));
            pending.erase(name);
            resource_assets.erase(name);
            return;
        }

        bool status = true;

        vm->create_environment(name);
        vm->pop();

        for (const auto& script : resource->scripts) {
            if (script.type != "shared" && script.type != "client") continue;

            const std::string asset_path = fmt::format("resources/{}/{}", name, script.src);

            std::string source;
            try {
                source = Vital::Tool::File::read_text(get_directory(), asset_path);
            }
            catch (...) {
                Vital::print("error", fmt::format("Resource `{}` failed to read script `{}`", name, script.src));
                status = false;
                break;
            }

            vm->get_reference(name, true);
            if (!vm->load_string(source, true, true, vm->get_count())) {
                Vital::print("error", fmt::format("Resource `{}` failed to execute script `{}`", name, script.src));
                vm->pop();
                status = false;
                break;
            }
            vm->pop();
        }

        pending.erase(name);
        resource_assets.erase(name);

        if (!status) {
            vm->clear_environment_id(name);
            Vital::print("error", fmt::format("Resource `{}` failed to load — env released", name));
            return;
        }

        running.insert(name);
        Vital::print("sbox", fmt::format("Resource `{}` loaded on client", name));
        Sandbox::get_singleton() -> signal("vital.resource:started", Vital::Tool::StackValue(name));
    }

    bool ResourceManager::unload(const std::string& name) {
        auto* vm = Sandbox::get_singleton() -> get_vm();
        auto* am = AssetManager::get_singleton();

        if (!is_running(name) && !is_pending(name)) {
            Vital::print("error", fmt::format("Cannot unload `{}` — not running or pending", name));
            return false;
        }

        if (is_pending(name)) {
            resource_assets.erase(name);
            pending.erase(name);
            am->cancel_group(name);
            Vital::print("sbox", fmt::format("Resource `{}` download cancelled", name));
        }

        if (is_running(name)) {
            vm->clear_environment_id(name);
            running.erase(name);
        }

        unregister_remote(name);
        Vital::print("sbox", fmt::format("Resource `{}` unloaded on client", name));
        Sandbox::get_singleton() -> signal("vital.resource:stopped", Vital::Tool::StackValue(name));
        return true;
    }

    #endif
}