/*----------------------------------------------------------------
     Resource: Vital.sandbox
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


    // Managers //
    void Resource::log(const std::string& mode, const std::string& message) {
        Tool::print(mode, fmt::format("Resource: {}", message));
    }


    // Checkers //
    bool Resource::is_name(const std::string& name) {
        if (name.empty() || !Tool::File::sanitize(name)) return false;
        for (const char c : name) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && (c != '_')) return false;
        }
        return true;
    }

    bool Resource::is_type(const std::string& type) {
        return (type == "shared") || (type == Tool::get_platform());
    }

    bool Resource::is_loaded(const std::string& name) const {
        return std::any_of(resources.begin(), resources.end(), [&](const Manifest& resource) {
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
        std::vector<const Manifest*> result;
        result.reserve(resources.size());
        for (const auto& resource : resources) result.push_back(&resource);
        return result;
    }

    const Resource::Manifest* Resource::get_resource(const std::string& name) const {
        for (const auto& resource : resources) {
            if (resource.ref == name) return &resource;
        }
        return nullptr;
    }

    std::string Resource::get_resource_from_vm(Vital::Sandbox::Machine* vm) {
        return vm -> get_environment_id();
    }

    std::string Resource::get_resource_base(const std::string& name, bool require_running) {
        if (!is_name(name)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error, "invalid resource name");
        if (require_running && !get_singleton() -> is_running(name)) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error, "resource not running");
        return Tool::get_directory("resources", name);
    }

    std::vector<Resource::Script> Resource::get_resource_scripts(const std::string& name, const std::string& type) const {
        std::vector<Script> result;
        const auto* resource = get_resource(name);
        if (!resource) return result;
        for (const auto& script : resource -> scripts) {
            if (type.empty() || script.type == type) result.push_back(script);
        }
        return result;
    }


    // APIs //
    #if !defined(Vital_SDK_Client)
    void Resource::broadcast_resource_event(const std::string& type, const std::string& name, const Manifest* manifest) const {
        Tool::Stack msg;
        msg.object["type"] = Tool::StackValue(type);
        msg.object["name"] = Tool::StackValue(name);
        if (manifest) {
            Tool::Stack scripts_stack;
            for (const auto& s : manifest -> scripts) {
                Tool::Stack entry;
                entry.object["src"]  = Tool::StackValue(s.src);
                entry.object["type"] = Tool::StackValue(s.type);
                scripts_stack.array.push_back(Tool::StackValue(std::move(entry)));
            }
            msg.object["scripts"] = Tool::StackValue(std::move(scripts_stack));

            Tool::Stack files_stack;
            for (const auto& f : manifest -> files)
                files_stack.array.push_back(Tool::StackValue(f));
            msg.object["files"] = Tool::StackValue(std::move(files_stack));
        }
        Vital::Engine::Network::get_singleton() -> broadcast(msg);
    }

    bool Resource::parse_manifest(Manifest& resource, Tool::YAML& manifest, const std::string& base, std::vector<std::string>& errors) {
        resource.name = manifest.get_str("name", resource.ref);
        resource.author = manifest.get_str("author", "");
        resource.version = manifest.get_str("version", "");
        resource.scripts.clear();
        resource.files.clear();
        resource.script_hashes.clear();
        resource.file_hashes.clear();
        if (!manifest.has("scripts") || !manifest.get_root()["scripts"].is_seq()) return false;

        for (ryml::ConstNodeRef node : manifest.get_root()["scripts"]) {
            if (!node.is_map() || !Tool::YAML::has(node, "src") || !Tool::YAML::has(node, "type")) { errors.push_back("script entry missing `src` or `type`"); continue; }
            const std::string src  = Tool::YAML::get_str(node, "src");
            const std::string type = Tool::YAML::get_str(node, "type");
            if (!Types.count(type)) { errors.push_back(fmt::format("script `{}` has invalid type `{}`", src, type)); continue; }
            auto expanded = Tool::File::glob(base, src);
            if (expanded.empty()) { errors.push_back(fmt::format("script pattern `{}` matched no files", src)); continue; }
            for (const auto& s : expanded) {
                resource.scripts.push_back({ s, type });
                resource.script_hashes[s] = Tool::File::hash(base, s);
            }
        }
        if (manifest.has("files") && manifest.get_root()["files"].is_seq()) {
            for (ryml::ConstNodeRef node : manifest.get_root()["files"]) {
                std::string file_pattern;
                node >> file_pattern;
                auto expanded = Tool::File::glob(base, file_pattern);
                if (expanded.empty()) { errors.push_back(fmt::format("file pattern `{}` matched no files", file_pattern)); continue; }
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
        std::lock_guard<std::mutex> lock(scan_mutex);
        log("sbox", "rescanning resources...");
        resources.clear();
        std::vector<std::string> contents;
        try { contents = Tool::File::contents(Tool::get_directory(), "resources", true); }
        catch (...) { log("error", "resource scan skipped — `resources/` directory not found"); return; }

        std::unordered_map<std::string, int> resource_count;
        for (const auto& path : contents) {
            resource_count[path.substr(path.find_last_of("/\\") + 1)]++;
        }

        auto try_read = [](const std::string& base, const std::string& file, std::string& out) -> bool {
            try { out = Tool::File::read_text(base, file); return true; }
            catch (...) { return false; }
        };

        for (const auto& path : contents) {
            const std::string name = path.substr(path.find_last_of("/\\") + 1);
            if (!is_name(name)) { log("error", fmt::format("invalid resource name `{}` — skipping", name)); continue; }
            if (resource_count[name] > 1) { log("error", fmt::format("duplicate resource found — skipping `{}`", name)); continue; }
            if (!Tool::File::exists(get_resource_base(name), "manifest.yaml")) continue;

            std::string content;
            if (!try_read(get_resource_base(name), "manifest.yaml", content)) { log("error", fmt::format("failed to read manifest for `{}` — skipping", name)); continue; }
            Tool::YAML manifest;
            try { manifest.parse(content); }
            catch (const std::exception& e) { log("error", fmt::format("malformed yaml in manifest for `{}` — skipping\n> {}", name, e.what())); continue; }
            Manifest resource;
            resource.ref = name;
            std::vector<std::string> errors;

            #if !defined(Vital_SDK_Client)
            if (!parse_manifest(resource, manifest, get_resource_base(name), errors)) {
            #else
            resource.name = manifest.get_str("name", name);
            resource.author = manifest.get_str("author", "");
            resource.version = manifest.get_str("version", "");
            if (false) {
            #endif
                if (!errors.empty()) {
                    std::string report = fmt::format("resource `{}` skipped:\n", name);
                    report += fmt::format("> Errors ({}):\n", errors.size());
                    for (const auto& err : errors) report += fmt::format("> {}\n", err);
                    log("error", report);
                }
                else log("error", fmt::format("resource `{}` has no valid `scripts` section — skipping", name));
                continue;
            }
            resources.push_back(std::move(resource));
        }

        std::string report = fmt::format("scan complete — {} resource(s) loaded\n", resources.size());
        for (const auto& resource : resources) report += fmt::format("> `{}`\n", resource.ref);
        log("sbox", report);

        #if !defined(Vital_SDK_Client)
        {
            std::vector<std::string> stale;
            for (const auto& name : running) {
                if (!is_loaded(name)) stale.push_back(name);
            }
            for (const auto& name : stale) {
                log("sbox", fmt::format("resource `{}` no longer exists — stopping", name));
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
            log("sbox", "initializing client resource manager...");

            Tool::Event::bind("asset:file_ready", [this](Tool::Stack args) {
                if (!args.object.count("path")) return;
                const std::string path = args.object.at("path").as<std::string>();
                auto* rm = Resource::get_singleton();
                for (auto& [name, remaining] : rm -> resource_assets) {
                    if (!remaining.count(path)) continue;
                    remaining.erase(path);
                    log("sbox", fmt::format("resource `{}` asset ready: {}", name, path));
                    // TODO: Utilize load() for this?? to register safely or send_running_resources_to_peer handles it?
                    if (remaining.empty()) rm -> execute_scripts(name);
                    break;
                }
            });

            Tool::Event::bind("network:packet", [this](Tool::Stack args) {
                if (!args.object.count("type")) return;
                const std::string type = args.object.at("type").as<std::string>();
                auto* rm = Resource::get_singleton();

                if (type == "vital.resource:started") {
                    if (!args.object.count("name")) return;
                    const std::string name = args.object.at("name").as<std::string>();
                    std::vector<Script> scripts;
                    std::vector<std::string> files;
                    unpack_manifest_payload(args, scripts, files);
                    log("sbox", fmt::format("client received resource start: `{}`", name));
                    if (!rm -> is_running(name) && !rm -> is_pending(name)) rm -> load(name, scripts, files);
                }
                else if (type == "vital.resource:stopped") {
                    if (!args.object.count("name")) return;
                    const std::string name = args.object.at("name").as<std::string>();
                    log("sbox", fmt::format("client received resource stop: `{}`", name));
                    rm -> unload(name);
                }
            });
            log("sbox", "client resource manager initialized");
        #else
            scan();
        #endif
    }


    #if !defined(Vital_SDK_Client)
    bool Resource::start(const std::string& name) {
        auto* vm = Manager::Sandbox::get_singleton() -> get_vm();
        auto* am = Manager::Asset::get_singleton();
        if (!is_loaded(name)) { log("error", fmt::format("cannot start `{}` — resource not loaded", name)); return false; }
        if (is_running(name)) { log("error", fmt::format("cannot start `{}` — already running", name)); return false; }

        const auto* resource = get_resource(name);
        std::vector<std::pair<std::string, std::string>> sources;
        std::vector<std::string> errors;
        for (const auto& script : resource -> scripts) {
            std::string source;
            try { source = Tool::File::read_text(get_resource_base(name), script.src); }
            catch (...) { errors.push_back(fmt::format("`{}` failed to read", script.src)); continue; }
            auto err = vm -> compile_string(source, script.src);
            if (!err.empty()) { errors.push_back(fmt::format("`{}` failed to compile ({})", script.src, err)); continue; }
            if (is_type(script.type)) sources.emplace_back(script.src, std::move(source));
        }
        if (!errors.empty()) {
            std::string report = fmt::format("resource `{}` failed to start\n", name);
            report += fmt::format("> Errors ({}):\n", errors.size());
            for (const auto& err : errors) report += fmt::format("> {}\n", err);
            log("error", report);
            return false;
        }

        std::vector<std::string> asset_paths;
        for (const auto& file : resource -> files) asset_paths.push_back(fmt::format("resources/{}/{}", name, file));
        for (const auto& script : resource -> scripts) {
            if (script.type == "shared" || script.type == "client") asset_paths.push_back(fmt::format("resources/{}/{}", name, script.src));
        }
        am -> register_assets(asset_paths, name);
        am -> broadcast_manifest_deferred();
        running.insert(name);
        log("sbox", fmt::format("resource `{}` started", name));

        vm -> create_environment(name);
        vm -> pop(1);
        for (const auto& [src, source] : sources) {
            vm -> get_reference(name, true);
            vm -> load_string(source, src, true, true, vm -> get_count());
            vm -> pop(1);
        }

        Engine::Core::get_singleton() -> push_deferred([this, name]() {
            broadcast_resource_event("vital.resource:started", name, get_resource(name));
            Manager::Sandbox::get_singleton() -> signal("vital.resource:started", Tool::StackValue(name));
        });
        return true;
    }

    bool Resource::stop(const std::string& name) {
        auto* vm = Manager::Sandbox::get_singleton() -> get_vm();
        auto* am = Manager::Asset::get_singleton();
        if (!is_running(name)) { log("error", fmt::format("cannot stop `{}` — not running", name)); return false; }

        am -> unregister_group(name);
        vm -> clear_environment_id(name);
        running.erase(name);
        log("sbox", fmt::format("resource `{}` stopped", name));

        Engine::Core::get_singleton() -> push_deferred([this, name]() {
            broadcast_resource_event("vital.resource:stopped", name);
            Manager::Sandbox::get_singleton() -> signal("vital.resource:stopped", Tool::StackValue(name));
        });
        return true;
    }

    bool Resource::restart(const std::string& name) {
        if (!is_running(name)) { log("error", fmt::format("cannot restart `{}` — not running", name)); return false; }

        stop(name);
        const std::string base = get_resource_base(name);
        if (!Tool::File::exists(base, "manifest.yaml")) {
            resources.erase(std::remove_if(resources.begin(), resources.end(), [&](const Manifest& m) { return m.ref == name; }), resources.end());
            log("error", fmt::format("resource `{}` manifest not found — unregistered", name));
            return false;
        }
        std::string content;
        try { content = Tool::File::read_text(base, "manifest.yaml"); }
        catch (...) { log("error", fmt::format("resource `{}` failed to read manifest — skipping restart", name)); return false; }
        Tool::YAML manifest;
        try { manifest.parse(content); }
        catch (const std::exception& e) { log("error", fmt::format("resource `{}` malformed yaml — {} — skipping restart", name, e.what())); return false; }

        for (auto& resource : resources) {
            if (resource.ref != name) continue;
            const auto old_script_hashes = resource.script_hashes;
            const auto old_file_hashes = resource.file_hashes;
            std::vector<std::string> errors;
            if (!parse_manifest(resource, manifest, base, errors)) {
                if (!errors.empty()) {
                    std::string report = fmt::format("resource `{}` restart aborted\n", name);
                    report += fmt::format("> Errors ({}):\n", errors.size());
                    for (const auto& err : errors) report += fmt::format("> {}\n", err);
                    log("error", report);
                }
                else log("error", fmt::format("resource `{}` has no valid `scripts` section — skipping restart", name));
                return false;
            }

            auto diff = [](const std::unordered_map<std::string, std::string>& old_map, const std::unordered_map<std::string, std::string>& new_map, const std::string& label, std::vector<std::string>& changes) {
                for (const auto& [k, v] : old_map) {
                    if (!new_map.count(k)) changes.push_back(fmt::format("`{}` ({} deleted)", k, label));
                    else if (new_map.at(k) != v) changes.push_back(fmt::format("`{}` ({} modified)", k, label));
                }
                for (const auto& [k, v] : new_map) {
                    if (!old_map.count(k)) changes.push_back(fmt::format("`{}` ({} added)", k, label));
                }
            };

            std::vector<std::string> changes;
            diff(old_script_hashes, resource.script_hashes, "script", changes);
            diff(old_file_hashes, resource.file_hashes, "file", changes);
            std::string report = fmt::format("resource `{}` restarted\n", name);
            if (changes.empty()) report += "> No changes detected";
            else {
                report += fmt::format("> Changes ({}):\n", changes.size());
                for (const auto& change : changes) report += fmt::format("> {}\n", change);
            }
            log("sbox", report);
            break;
        }
        return start(name);
    }

    void Resource::start_all() {
        log("sbox", "starting all resources...");
        int count = 0;
        for (const auto* resource : get_all_resources()) {
            if (start(resource -> ref)) count++;
        }
        log("sbox", fmt::format("all resources started — {} resource(s) running", count));
    }

    void Resource::stop_all() {
        log("sbox", "stopping all resources...");
        std::unordered_set<std::string> snapshot = running;
        int count = 0;
        for (const auto& name : snapshot) {
            if (stop(name)) count++;
        }
        log("sbox", fmt::format("all resources stopped — {} resource(s) stopped", count));
    }

    void Resource::restart_all() {
        log("sbox", "restarting all resources...");
        std::unordered_set<std::string> snapshot = running;
        int count = 0;
        for (const auto& name : snapshot) {
            if (restart(name)) count++;
        }
        log("sbox", fmt::format("all resources restarted — {} resource(s) restarted", count));
    }
    #endif


    #if defined(Vital_SDK_Client)
    void Resource::unpack_manifest_payload(const Tool::Stack& args, std::vector<Script>& scripts, std::vector<std::string>& files) const {
        if (const auto* sv = args.get("scripts")) {
            const auto& nested = *sv -> as<std::shared_ptr<Tool::Stack>>();
            scripts.reserve(nested.array.size());
            for (const auto& entry : nested.array) {
                const auto& s = *entry.as<std::shared_ptr<Tool::Stack>>();
                scripts.push_back({ s.object.at("src").as<std::string>(), s.object.at("type").as<std::string>() });
            }
        }
        if (const auto* sv = args.get("files")) {
            const auto& nested = *sv -> as<std::shared_ptr<Tool::Stack>>();
            files.reserve(nested.array.size());
            for (const auto& entry : nested.array) files.push_back(entry.as<std::string>());
        }
    }

    bool Resource::load(const std::string& name, const std::vector<Script>& scripts, const std::vector<std::string>& files) {
        if (is_running(name) || is_pending(name)) { log("error", fmt::format("cannot load `{}` — already running or pending", name)); return false; }

        resources.erase(std::remove_if(resources.begin(), resources.end(), [&](const Manifest& m) { return m.ref == name; }), resources.end());
        Manifest manifest;
        manifest.ref = name;
        manifest.name = name;
        manifest.scripts = scripts;
        manifest.files = files;
        resources.push_back(std::move(manifest));
        log("sbox", fmt::format("resource `{}` registered from server — {} script(s), {} file(s)", name, scripts.size(), files.size()));

        const auto* resource = get_resource(name);
        std::unordered_set<std::string> asset_paths;
        for (const auto& file : resource -> files) asset_paths.insert(fmt::format("resources/{}/{}", name, file));
        for (const auto& script : resource -> scripts) {
            if (is_type(script.type)) asset_paths.insert(fmt::format("resources/{}/{}", name, script.src));
        }
        pending.insert(name);
        for (const auto& path : asset_paths) {
            if (Tool::File::exists(Tool::get_directory(), path)) log("sbox", fmt::format("resource `{}` asset cached: {}", name, path));
            else resource_assets[name].insert(path);
        }

        log("sbox", fmt::format("resource `{}` queued — {} asset(s) pending download", name, resource_assets[name].size()));
        if (resource_assets[name].empty()) {
            log("sbox", fmt::format("resource `{}` all assets cached — executing immediately", name));
            execute_scripts(name);
        }
        return true;
    }

    void Resource::execute_scripts(const std::string& name) {
        if (!is_pending(name)) return;
        auto* vm = Manager::Sandbox::get_singleton() -> get_vm();
        const auto* resource = get_resource(name);
        if (!resource) {
            log("error", fmt::format("execute_scripts: manifest null for `{}` — aborting", name));
            pending.erase(name);
            resource_assets.erase(name);
            return;
        }

        std::vector<std::pair<std::string, std::string>> sources;
        std::vector<std::string> errors;
        for (const auto& script : resource -> scripts) {
            if (script.type != "shared" && script.type != "client") continue;
            std::string source;
            try { source = Tool::File::read_text(Tool::get_directory(), fmt::format("resources/{}/{}", name, script.src)); }
            catch (...) { errors.push_back(fmt::format("`{}` failed to read", script.src)); continue; }
            auto err = vm -> compile_string(source, script.src);
            if (!err.empty()) { errors.push_back(fmt::format("`{}` failed to compile ({})", script.src, err)); continue; }
            sources.emplace_back(script.src, std::move(source));
        }

        pending.erase(name);
        resource_assets.erase(name);
        if (!errors.empty()) {
            std::string report = fmt::format("resource `{}` failed to load\n", name);
            report += fmt::format("> Errors ({}):\n", errors.size());
            for (const auto& err : errors) report += fmt::format("> {}\n", err);
            log("error", report);
            return;
        }

        vm -> create_environment(name);
        vm -> pop(1);
        for (const auto& [src, source] : sources) {
            vm -> get_reference(name, true);
            vm -> load_string(source, src, true, true, vm -> get_count());
            vm -> pop(1);
        }
        running.insert(name);
        log("sbox", fmt::format("resource `{}` loaded on client", name));
        Manager::Sandbox::get_singleton() -> signal("vital.resource:started", Tool::StackValue(name));
    }

    bool Resource::unload(const std::string& name) {
        auto* vm = Manager::Sandbox::get_singleton() -> get_vm();
        auto* am = Manager::Asset::get_singleton();
        if (!is_running(name) && !is_pending(name)) { log("error", fmt::format("cannot unload `{}` — not running or pending", name)); return false; }
        if (is_pending(name)) {
            resource_assets.erase(name);
            pending.erase(name);
            am -> cancel_group(name);
            log("sbox", fmt::format("resource `{}` download cancelled", name));
        }

        if (is_running(name)) { vm -> clear_environment_id(name); running.erase(name); }
        resources.erase(std::remove_if(resources.begin(), resources.end(), [&](const Manifest& m) { return m.ref == name; }), resources.end());
        log("sbox", fmt::format("resource `{}` unloaded on client", name));
        Manager::Sandbox::get_singleton() -> signal("vital.resource:stopped", Tool::StackValue(name));
        return true;
    }
    #endif
}