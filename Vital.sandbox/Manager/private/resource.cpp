/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: resource.cpp
     Script: Manager: resource.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Resource Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#include <Vital.sandbox/Manager/public/resource.h>
#include <Vital.sandbox/Engine/public/console.h>
#include <Vital.sandbox/Engine/public/network.h>
#include <Vital.sandbox/Manager/public/asset.h>
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Tool/yaml.h>


///////////////////////////////
// Vital: Manager: Resource //
///////////////////////////////

namespace Vital::Manager {
    // TODO: Improve

    // Utils //
    Resource* Resource::get_singleton() {
        if (!singleton) singleton = new Resource();
        return singleton;
    }

    void Resource::free_singleton() {
        if (!singleton) return;
        delete singleton;
        singleton = nullptr;
    }


    // Checkers //
    bool Resource::is_name(const std::string& name) {
        if (name.empty() || !Tool::File::sanitize(name)) return false;
        for (const char c : name) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && (c != '_'))
                return false;
        }
        return true;
    }

    bool Resource::is_type(const std::string& type) {
        return (type == "shared") || (type == Tool::get_platform());
    }

    bool Resource::is_loaded(const std::string& name) const {
        return std::any_of(resources.begin(), resources.end(), [&](const Manifest& r) { 
            return resource.ref == name; 
        });
    }

    bool Resource::is_running(const std::string& name) const {
        return running.count(name) > 0;
    }

    #if defined(Vital_SDK_Client)
    bool Resource::is_pending(const std::string& name) const {
        return pending.count(name) > 0;
    }
    #endif


    // Getters //
    std::vector<const Resource::Manifest*> Resource::get_all_resources() const {
        std::vector<const Resource::Manifest*> result;
        result.reserve(resources.size());
        for (const auto& resource : resources)
            result.push_back(&resource);
        return result;
    }

    const Resource::Manifest* Resource::get_resource(const std::string& name) const {
        for (const auto& resource : resources)
            if (resource.ref == name) return &resource;
        return nullptr;
    }

    std::string Resource::get_resource_from_vm(Vital::Sandbox::Machine* vm) {
        return vm->get_environment_id();
    }

    std::string Resource::get_resource_base(const std::string& name, bool require_running) {
        if (!is_name(name)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error, "invalid resource name");
        if (require_running && !get_singleton() -> is_running(name)) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error, "resource not running");
        return Tool::get_directory("resources", name);
    }

    std::vector<Resource::Script> Resource::get_resource_scripts(const std::string& name, const std::string& type) const {
        std::vector<Resource::Script> result;
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
    bool Resource::parse_manifest(Manifest& resource, Tool::YAML& manifest, const std::string& base, std::vector<std::string>& errors) {
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
            if (!node.is_map() || !Tool::YAML::has(node, "src") || !Tool::YAML::has(node, "type")) {
                errors.push_back("script entry missing `src` or `type`");
                continue;
            }
            const std::string src  = Tool::YAML::get_str(node, "src");
            const std::string type = Tool::YAML::get_str(node, "type");
            if (Types.find(type) == Types.end()) {
                errors.push_back(fmt::format("script `{}` has invalid type `{}`", src, type));
                continue;
            }
            std::vector<std::string> expanded = Tool::File::glob(base, src);
            if (expanded.empty()) {
                errors.push_back(fmt::format("script pattern `{}` matched no files", src));
                continue;
            }
            for (const auto& s : expanded) {
                resource.scripts.push_back({ s, type });
                resource.script_hashes[s] = Tool::File::hash(base, s);
            }
        }

        if (manifest.has("files") && manifest.get_root()["files"].is_seq()) {
            for (ryml::ConstNodeRef node : manifest.get_root()["files"]) {
                std::string file_pattern;
                node >> file_pattern;
                std::vector<std::string> expanded = Tool::File::glob(base, file_pattern);
                if (expanded.empty()) {
                    errors.push_back(fmt::format("file pattern `{}` matched no files", file_pattern));
                    continue;
                }
                for (const auto& f : expanded) {
                    resource.files.push_back(f);
                    resource.file_hashes[f] = Tool::File::hash(base, f);
                }
            }
        }

        return errors.empty();
    }

    #endif

    void Resource::scan() {
        Tool::print("sbox", "Rescanning resources...");
        resources.clear();

        std::vector<std::string> contents;
        try {
            contents = Tool::File::contents(Tool::get_directory(), "resources", true);
        }
        catch (...) {
            Tool::print("error", "Resource scan skipped — `resources/` directory not found");
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
                Tool::print("error", fmt::format("Invalid resource name `{}` — skipping", name));
                continue;
            }
            if (resource_count[name] > 1) {
                Tool::print("error", fmt::format("Duplicate resource found — skipping `{}`", name));
                continue;
            }
            if (!Tool::File::exists(get_resource_base(name), "manifest.yaml")) continue;

            std::string content;
            try {
                content = Tool::File::read_text(get_resource_base(name), "manifest.yaml");
            }
            catch (...) {
                Tool::print("error", fmt::format("Failed to read manifest for `{}` — skipping", name));
                continue;
            }

            Tool::YAML manifest;
            try {
                manifest.parse(content);
            }
            catch (const std::exception& e) {
                Tool::print("error", fmt::format(
                    "Malformed YAML in manifest for `{}` — skipping\n"
                    "> {}",
                    name, e.what()
                ));
                continue;
            }

            Manifest resource;
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
                    Tool::print("error", error_list);
                }
                else {
                    Tool::print("error", fmt::format("Resource `{}` has no valid `scripts` section — skipping", name));
                }
                continue;
            }
            resources.push_back(std::move(resource));
        }

        // Report
        std::string report = fmt::format("Resource scan complete — {} resource(s) loaded\n", resources.size());
        for (const auto& resource : resources)
            report += fmt::format("> `{}`\n", resource.ref);
        Tool::print("sbox", report);

        #if !defined(Vital_SDK_Client)
        {
            std::vector<std::string> stale;
            for (const auto& name : running) {
                if (!is_loaded(name)) stale.push_back(name);
            }
            for (const auto& name : stale) {
                Tool::print("sbox", fmt::format("Resource `{}` no longer exists — stopping", name));
                stop(name);
            }
        }

        static bool server_bound = false;
        if (!server_bound) {
            server_bound = true;
            Tool::Event::bind("vital.network:peer_connected", [](Tool::Stack args) {
                if (args.array.empty()) return;
                const int peer_id = args.array[0].as<int32_t>();
                Engine::Core::get_singleton() -> push_deferred([peer_id]() {
                    Manager::Asset::get_singleton() -> broadcast_manifest(peer_id);
                });
            });
        }
        #endif
    }

    void Resource::init() {
        #if defined(Vital_SDK_Client)
            static bool client_initialized = false;
            if (client_initialized) return;
            client_initialized = true;

            Tool::print("sbox", "Initializing client resource manager...");

            Tool::Event::bind("asset:file_ready", [](Tool::Stack args) {
                if (!args.object.count("path")) return;
                const std::string path = args.object.at("path").as<std::string>();
                auto* rm = Resource::get_singleton();
                for (auto& [name, remaining] : rm->resource_assets) {
                    if (!remaining.count(path)) continue;
                    remaining.erase(path);
                    Tool::print("sbox", fmt::format("Resource `{}` asset ready: {}", name, path));
                    if (remaining.empty()) rm->execute_scripts(name);
                    break;
                }
            });

            Tool::Event::bind("network:packet", [](Tool::Stack args) {
                if (!args.object.count("type")) return;
                const std::string type = args.object.at("type").as<std::string>();
                auto* rm = Resource::get_singleton();

                if (type == "vital.resource:started") {
                    if (!args.object.count("name")) return;
                    const std::string name = args.object.at("name").as<std::string>();

                    std::vector<Resource::Script> scripts;
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
                    Tool::print("sbox", fmt::format("Client received resource start: `{}`", name));
                    if (!rm->is_running(name) && !rm->is_pending(name)) rm->load(name);
                }
                else if (type == "vital.resource:stopped") {
                    if (!args.object.count("name")) return;
                    const std::string name = args.object.at("name").as<std::string>();
                    Tool::print("sbox", fmt::format("Client received resource stop: `{}`", name));
                    rm->unload(name);
                }
            });
            Tool::print("sbox", "Client resource manager initialized");
        #else
            scan();
        #endif
    }


    #if !defined(Vital_SDK_Client)

    void Resource::notify_resource_started(const std::string& name) {
        const auto* resource = get_resource(name);
        if (!resource) return;

        Tool::Stack msg;
        msg.object["type"] = Tool::StackValue(std::string("vital.resource:started"));
        msg.object["name"] = Tool::StackValue(name);

        msg.object["script_count"] = Tool::StackValue((int32_t)resource->scripts.size());
        for (int i = 0; i < (int)resource->scripts.size(); i++) {
            msg.object[fmt::format("script_src_{}", i)]  = Tool::StackValue(resource->scripts[i].src);
            msg.object[fmt::format("script_type_{}", i)] = Tool::StackValue(resource->scripts[i].type);
        }

        msg.object["file_count"] = Tool::StackValue((int32_t)resource->files.size());
        for (int i = 0; i < (int)resource->files.size(); i++) {
            msg.object[fmt::format("file_{}", i)] = Tool::StackValue(resource->files[i]);
        }

        for (int peer_id : Vital::Engine::Network::get_singleton() -> get_connected_peers()) {
            Vital::Engine::Network::get_singleton() -> send(msg, peer_id);
        }
    }

    void Resource::notify_resource_stopped(const std::string& name) {
        Tool::Stack msg;
        msg.object["type"] = Tool::StackValue(std::string("vital.resource:stopped"));
        msg.object["name"] = Tool::StackValue(name);
        for (int peer_id : Vital::Engine::Network::get_singleton() -> get_connected_peers()) {
            Vital::Engine::Network::get_singleton() -> send(msg, peer_id);
        }
    }

    bool Resource::start(const std::string& name) {
        auto* vm = Manager::Sandbox::get_singleton() -> get_vm();
        auto* am = Manager::Asset::get_singleton();

        if (!is_loaded(name)) {
            Tool::print("error", fmt::format("Cannot start `{}` — resource not loaded", name));
            return false;
        }
        if (is_running(name)) {
            Tool::print("error", fmt::format("Cannot start `{}` — already running", name));
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
        Tool::print("sbox", fmt::format("Resource `{}` started", name));
        Engine::Core::get_singleton() -> push_deferred([this, name]() {
            notify_resource_started(name);
        });
        Manager::Sandbox::get_singleton() -> signal("vital.resource:started", Tool::StackValue(name));

        // Phase 3: Execute scripts
        vm->create_environment(name);
        vm->pop();

        for (const auto& script : resource->scripts) {
            if (!is_type(script.type)) continue;

            std::string source;
            try {
                source = Tool::File::read_text(get_resource_base(name), script.src);
            }
            catch (...) {
                Tool::print("error", fmt::format("Resource `{}` failed to read script `{}`", name, script.src));
                running.erase(name);
                vm->clear_environment_id(name);
                am->unregister_group(name);
                return false;
            }

            vm->get_reference(name, true);
            if (!vm->load_string(source, true, true, vm->get_count())) {
                Tool::print("error", fmt::format("Resource `{}` failed to execute script `{}`", name, script.src));
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

    bool Resource::stop(const std::string& name) {
        auto* vm = Manager::Sandbox::get_singleton() -> get_vm();
        auto* am = Manager::Asset::get_singleton();
        
        if (!is_running(name)) {
            Tool::print("error", fmt::format("Cannot stop `{}` — not running", name));
            return false;
        }

        am->unregister_group(name);
        Engine::Core::get_singleton() -> push_deferred([this, name]() {
            notify_resource_stopped(name);
        });

        Manager::Sandbox::get_singleton() -> signal("vital.resource:stopped", Tool::StackValue(name));
        vm->clear_environment_id(name);

        running.erase(name);
        Tool::print("sbox", fmt::format("Resource `{}` stopped", name));
        return true;
    }

    bool Resource::restart(const std::string& name) {
        if (!is_running(name)) {
            Tool::print("error", fmt::format("Cannot restart `{}` — not running", name));
            return false;
        }

        stop(name);
        const std::string base = get_resource_base(name);
        if (!Tool::File::exists(base, "manifest.yaml")) {
            resources.erase(
                std::remove_if(resources.begin(), resources.end(),
                    [&name](const Manifest& m) { return m.ref == name; }),
                resources.end()
            );
            Tool::print("error", fmt::format("Resource `{}` manifest not found — unregistered", name));
            return false;
        }

        std::string content;
        try {
            content = Tool::File::read_text(base, "manifest.yaml");
        }
        catch (...) {
            Tool::print("error", fmt::format("Resource `{}` failed to read manifest — skipping restart", name));
            return false;
        }

        Tool::YAML manifest;
        try {
            manifest.parse(content);
        }
        catch (const std::exception& e) {
            Tool::print("error", fmt::format("Resource `{}` malformed YAML — {} — skipping restart", name, e.what()));
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
                    Tool::print("error", error_list);
                }
                else {
                    Tool::print("error", fmt::format("Resource `{}` has no valid `scripts` section — skipping restart", name));
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
            Tool::print("sbox", report);

            break;
        }

        return start(name);
    }

    void Resource::start_all() {
        Tool::print("sbox", "Starting all resources...");
        int count = 0;
        for (const auto* resource : get_all_resources())
            if (start(resource->ref)) count++;
        Tool::print("sbox", fmt::format("All resources started — {} resource(s) running", count));
    }
    
    void Resource::stop_all() {
        Tool::print("sbox", "Stopping all resources...");
        std::unordered_set<std::string> snapshot = running;
        int count = 0;
        for (const auto& name : snapshot)
            if (stop(name)) count++;
        Tool::print("sbox", fmt::format("All resources stopped — {} resource(s) stopped", count));
    }
    
    void Resource::restart_all() {
        Tool::print("sbox", "Restarting all resources...");
        std::unordered_set<std::string> snapshot = running;
        int count = 0;
        for (const auto& name : snapshot)
            if (restart(name)) count++;
        Tool::print("sbox", fmt::format("All resources restarted — {} resource(s) restarted", count));
    }
    #endif


    #if defined(Vital_SDK_Client)

    bool Resource::register_remote(const std::string& name, const std::vector<Script>& scripts, const std::vector<std::string>& files) {
        unregister_remote(name);
        Manifest manifest;
        manifest.ref     = name;
        manifest.name    = name;
        manifest.scripts = scripts;
        manifest.files   = files;
        resources.push_back(std::move(manifest));
        Tool::print("sbox", fmt::format("Resource `{}` manifest registered from server — {} script(s), {} file(s)", name, scripts.size(), files.size()));
        return true;
    }

    void Resource::unregister_remote(const std::string& name) {
        resources.erase(
            std::remove_if(resources.begin(), resources.end(),
                [&name](const Manifest& m) { return m.ref == name; }),
            resources.end()
        );
    }

    bool Resource::load(const std::string& name) {
        auto* am = Manager::Asset::get_singleton();

        if (!is_loaded(name)) {
            Tool::print("error", fmt::format("Cannot load `{}` — resource not registered", name));
            return false;
        }
        if (is_running(name) || is_pending(name)) {
            Tool::print("error", fmt::format("Cannot load `{}` — already running or pending", name));
            return false;
        }

        const auto* resource = get_resource(name);
        if (!resource) {
            Tool::print("error", fmt::format("Cannot load `{}` — manifest is null", name));
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
            if (Tool::File::exists(Tool::get_directory(), path)) {
                Tool::print("sbox", fmt::format("Resource `{}` asset cached: {}", name, path));
            } else {
                resource_assets[name].insert(path);
            }
        }

        Tool::print("sbox", fmt::format("Resource `{}` queued — {} asset(s) pending download", name, resource_assets[name].size()));

        if (resource_assets[name].empty()) {
            Tool::print("sbox", fmt::format("Resource `{}` all assets cached — executing immediately", name));
            execute_scripts(name);
        }

        return true;
    }

    void Resource::execute_scripts(const std::string& name) {
        if (!is_pending(name)) return;

        auto* vm             = Manager::Sandbox::get_singleton() -> get_vm();
        auto* am             = Manager::Asset::get_singleton();
        const auto* resource = get_resource(name);

        if (!resource) {
            Tool::print("error", fmt::format("execute_scripts: manifest null for `{}` — aborting", name));
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
                source = Tool::File::read_text(Tool::get_directory(), asset_path);
            }
            catch (...) {
                Tool::print("error", fmt::format("Resource `{}` failed to read script `{}`", name, script.src));
                status = false;
                break;
            }

            vm->get_reference(name, true);
            if (!vm->load_string(source, true, true, vm->get_count())) {
                Tool::print("error", fmt::format("Resource `{}` failed to execute script `{}`", name, script.src));
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
            Tool::print("error", fmt::format("Resource `{}` failed to load — env released", name));
            return;
        }

        running.insert(name);
        Tool::print("sbox", fmt::format("Resource `{}` loaded on client", name));
        Manager::Sandbox::get_singleton() -> signal("vital.resource:started", Tool::StackValue(name));
    }

    bool Resource::unload(const std::string& name) {
        auto* vm = Manager::Sandbox::get_singleton() -> get_vm();
        auto* am = Manager::Asset::get_singleton();

        if (!is_running(name) && !is_pending(name)) {
            Tool::print("error", fmt::format("Cannot unload `{}` — not running or pending", name));
            return false;
        }

        if (is_pending(name)) {
            resource_assets.erase(name);
            pending.erase(name);
            am->cancel_group(name);
            Tool::print("sbox", fmt::format("Resource `{}` download cancelled", name));
        }

        if (is_running(name)) {
            vm->clear_environment_id(name);
            running.erase(name);
        }

        unregister_remote(name);
        Tool::print("sbox", fmt::format("Resource `{}` unloaded on client", name));
        Manager::Sandbox::get_singleton() -> signal("vital.resource:stopped", Tool::StackValue(name));
        return true;
    }

    #endif
}