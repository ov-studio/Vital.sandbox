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

        #if !defined(Vital_SDK_Client)
        static bool server_bound = false;
        if (!server_bound) {
            server_bound = true;
            Vital::Tool::Event::bind("vital.network:peer_connected", [](Vital::Tool::Stack args) -> void {
                if (args.array.empty()) return;
                const int peer_id = args.array[0].as<int32_t>();
                Core::get_singleton() -> call_deferred("broadcast_asset_manifest", peer_id);
            });
        }
        #endif

        #if defined(Vital_SDK_Client)
        static bool client_bound = false;
        if (!client_bound) {
            client_bound = true;

            // Global asset:file_ready — routes each completed download to
            // the resource waiting for it, executes scripts when all land
            Vital::Tool::Event::bind("asset:file_ready", [](Vital::Tool::Stack args) -> void {
                if (!args.object.count("path")) return;
                const std::string path = args.object.at("path").as<std::string>();
                auto* rm = ResourceManager::get_singleton();
                for (auto& [name, remaining] : rm->resource_assets) {
                    if (!remaining.count(path)) continue;
                    remaining.erase(path);
                    Vital::print("sbox", "Resource `" + name + "` asset ready: " + path);
                    if (remaining.empty()) rm->execute_scripts(name);
                    break;
                }
            });

            // All incoming network packets arrive as "network:packet" —
            // filter by type to handle resource lifecycle messages from server.
            //
            // Scripts arrive as a nested Stack array where each entry is a Stack
            // object with "src" and "type" string keys — no "src|type" encoding needed.
            // Files arrive as a nested Stack array of plain strings.
            Vital::Tool::Event::bind("network:packet", [](Vital::Tool::Stack args) -> void {
                Vital::print("info", "received it!");
                if (!args.object.count("type")) return;
                const std::string type = args.object.at("type").as<std::string>();
                auto* rm = ResourceManager::get_singleton();

                if (type == "vital.resource:started") {
                    if (!args.object.count("name")) return;
                    const std::string name = args.object.at("name").as<std::string>();

                    // Decode scripts from nested Stack: array of { src, type } objects
                    std::vector<ResourceManager::ResourceScript> scripts;
                    if (args.object.count("scripts")) {
                        const auto& scripts_sv = args.object.at("scripts");
                        if (scripts_sv.is<std::shared_ptr<Vital::Tool::Stack>>()) {
                            const auto& scripts_stack = *scripts_sv.as<std::shared_ptr<Vital::Tool::Stack>>();
                            for (const auto& entry_sv : scripts_stack.array) {
                                if (!entry_sv.is<std::shared_ptr<Vital::Tool::Stack>>()) continue;
                                const auto& entry = *entry_sv.as<std::shared_ptr<Vital::Tool::Stack>>();
                                if (!entry.object.count("src") || !entry.object.count("type")) continue;
                                scripts.push_back({
                                    entry.object.at("src").as<std::string>(),
                                    entry.object.at("type").as<std::string>()
                                });
                            }
                        }
                    }

                    // Decode files from nested Stack: array of plain strings
                    std::vector<std::string> files;
                    if (args.object.count("files")) {
                        const auto& files_sv = args.object.at("files");
                        if (files_sv.is<std::shared_ptr<Vital::Tool::Stack>>()) {
                            const auto& files_stack = *files_sv.as<std::shared_ptr<Vital::Tool::Stack>>();
                            files.reserve(files_stack.array.size());
                            for (const auto& f : files_stack.array) {
                                if (f.is<std::string>()) files.push_back(f.as<std::string>());
                            }
                        }
                    }

                    // Register the remote manifest so is_loaded() and get_resource()
                    // are valid before load() is called — prevents nullptr deref crash
                    rm->register_remote(name, scripts, files);

                    Vital::print("sbox", "Client received resource start: `" + name + "`");
                    if (!rm->is_running(name) && !rm->is_pending(name)) rm->load(name);
                }
                else if (type == "vital.resource:stopped") {
                    if (!args.object.count("name")) return;
                    const std::string name = args.object.at("name").as<std::string>();
                    Vital::print("sbox", "Client received resource stop: `" + name + "`");
                    rm->unload(name);
                }
            });
        }
        #endif
    }


    #if !defined(Vital_SDK_Client)

    // Called deferred from Core — safe to call Network::send on main thread.
    //
    // Scripts are serialized as a nested Stack: an array of Stack objects,
    // each holding "src" and "type" string keys. Files are a nested Stack
    // array of plain strings. No "src|type" string-packing needed.
    void ResourceManager::notify_resource_started(const std::string& name) {
        const auto* resource = get_resource(name);
        if (!resource) return;

        Vital::print("info", "this caused crashed");
        Vital::Tool::Stack msg;
        msg.object["type"] = Vital::Tool::StackValue(std::string("vital.resource:started"));
        msg.object["name"] = Vital::Tool::StackValue(name);

        // Encode scripts as a nested Stack: array of { src, type } Stack objects
        Vital::Tool::Stack scripts_stack;
        scripts_stack.array.reserve(resource->scripts.size());
        for (const auto& script : resource->scripts) {
            Vital::Tool::Stack entry;
            entry.object["src"]  = Vital::Tool::StackValue(script.src);
            entry.object["type"] = Vital::Tool::StackValue(script.type);
            scripts_stack.array.emplace_back(std::move(entry));
        }
        msg.object["scripts"] = Vital::Tool::StackValue(std::move(scripts_stack));

        // Encode files as a nested Stack: array of plain strings
        Vital::Tool::Stack files_stack;
        files_stack.array.reserve(resource->files.size());
        for (const auto& file : resource->files)
            files_stack.array.emplace_back(file);
        msg.object["files"] = Vital::Tool::StackValue(std::move(files_stack));

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

        for (const auto& file : resource->files) {
            am->register_asset("resources/" + name + "/" + file);
        }
        for (const auto& script : resource->scripts) {
            if (script.type == "client" || script.type == "shared") {
                am->register_asset("resources/" + name + "/" + script.src);
            }
        }

        // Deferred — both send() calls must run on the main thread
        am->broadcast_manifest_deferred();

        Core::get_singleton() -> call_deferred(
            "notify_resource_started",
            godot::String(name.c_str())
        );
    
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

        // Notify clients deferred — must run on main thread
        Core::get_singleton() -> call_deferred(
            "notify_resource_stopped",
            godot::String(name.c_str())
        );

        Sandbox::get_singleton() -> signal("vital.resource:stopped", Vital::Tool::StackValue(name));
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

    // Registers a manifest received from the server into the local resources list.
    // Must be called before load() so that is_loaded() and get_resource() are valid.
    bool ResourceManager::register_remote(const std::string& name,
                                          const std::vector<ResourceScript>& scripts,
                                          const std::vector<std::string>& files) {
        if (is_loaded(name)) return false;
        ResourceManifest manifest;
        manifest.ref     = name;
        manifest.name    = name;
        manifest.scripts = scripts;
        manifest.files   = files;
        resources.push_back(std::move(manifest));
        Vital::print("sbox", "Resource `" + name + "` manifest registered from server");
        return true;
    }

    // Removes a server-registered manifest from the local resources list.
    // Called by unload() so a future restart receives a fresh manifest from
    // the server rather than hitting the is_loaded() guard in register_remote().
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
            Vital::print("error", "Cannot load `" + name + "` — resource not registered");
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

        pending.insert(name);

        // Pre-check which assets are already cached — only wait for
        // ones that genuinely need downloading
        for (const auto& path : asset_paths) {
            if (Vital::Tool::File::exists(am->get_local_base(), path)) {
                Vital::print("sbox", "Resource `" + name + "` asset cached: " + path);
            } else {
                resource_assets[name].insert(path);
            }
        }

        Vital::print("sbox", "Resource `" + name + "` queued — " + std::to_string(resource_assets[name].size()) + " asset(s) pending download");

        // All cached — execute immediately without waiting for any events
        if (resource_assets[name].empty()) {
            Vital::print("sbox", "Resource `" + name + "` all assets cached — executing immediately");
            execute_scripts(name);
        }

        return true;
    }

    void ResourceManager::execute_scripts(const std::string& name) {
        if (!is_pending(name)) return;

        auto* vm              = Sandbox::get_singleton() -> get_vm();
        auto* am              = AssetManager::get_singleton();
        const auto* resource  = get_resource(name);
        const std::string env = get_resource_env(name);
        bool status           = true;

        vm->create_environment();
        vm->set_reference(env);

        for (const auto& script : resource->scripts) {
            if (script.type != "shared" && script.type != "client") continue;

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

        pending.erase(name);
        resource_assets.erase(name);

        if (!status) {
            vm->del_reference(env);
            Vital::print("error", "Resource `" + name + "` failed to load — env released");
            return;
        }

        running.insert(name);
        Vital::print("sbox", "Resource `" + name + "` loaded on client");
        Sandbox::get_singleton() -> signal("vital.resource:started", Vital::Tool::StackValue(name));
    }

    bool ResourceManager::unload(const std::string& name) {
        auto* vm = Sandbox::get_singleton() -> get_vm();
        auto* am = AssetManager::get_singleton();

        if (!is_running(name) && !is_pending(name)) {
            Vital::print("error", "Cannot unload `" + name + "` — not running or pending");
            return false;
        }

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

        if (is_running(name)) {
            vm->del_reference(get_resource_env(name));
            running.erase(name);
        }

        // Remove the server-registered manifest so a future restart
        // gets a fresh one from the server rather than stale data
        unregister_remote(name);

        Vital::print("sbox", "Resource `" + name + "` unloaded on client");
        Sandbox::get_singleton() -> signal("vital.resource:stopped", Vital::Tool::StackValue(name));
        return true;
    }

    #endif
}