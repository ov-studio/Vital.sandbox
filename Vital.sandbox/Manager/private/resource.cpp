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


///////////////////////////////
// Vital: Manager: Resource //
///////////////////////////////

namespace Vital::Manager {
    // Utils //
    Resource* Resource::get_singleton() {
        if (!singleton) {
            singleton = new Resource();
            singleton -> ready();
        }
        return singleton;
    }

    void Resource::free_singleton() {
        if (!singleton) return;
        delete singleton;
        singleton = nullptr;
    }


    // Helpers //
    std::string Resource::chunk_name(const std::string& resource, const std::string& src) const {
        return fmt::format("@{}/{}", resource, src);
    }

    Tool::Stack Resource::pack_manifest(const Manifest& manifest) const {
        Tool::Stack out;
        Tool::Stack scripts_stack;
        for (const auto& s : manifest.scripts) {
            Tool::Stack entry;
            entry.object["src"] = Tool::StackValue(s.src);
            entry.object["type"] = Tool::StackValue(s.type);
            scripts_stack.array.push_back(Tool::StackValue(std::move(entry)));
        }
        out.object["scripts"] = Tool::StackValue(std::move(scripts_stack));
        Tool::Stack files_stack;
        for (const auto& f : manifest.files) files_stack.array.push_back(Tool::StackValue(f));
        out.object["files"] = Tool::StackValue(std::move(files_stack));
        return out;
    }

    void Resource::unpack_manifest(const Tool::Stack& args, std::vector<Script>& scripts, std::vector<std::string>& files) const {
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

    bool Resource::validate_scripts(const std::string& name, std::vector<std::pair<std::string, std::string>>& sources) const {
        auto vm = Manager::Sandbox::get_singleton() -> get_vm();
        const Manifest* resource;
        {
            std::lock_guard<std::mutex> lock(mutex);
            resource = get_resource_unsafe(name);
        }
        std::vector<std::string> errors;
        for (const auto& script : resource -> scripts) {
            #if defined(Vital_SDK_Client)
            if (!is_type(script.type)) continue;
            #endif
            std::string source;
            try { source = Tool::File::read_text(get_resource_base(name), script.src); }
            catch (...) { errors.push_back(fmt::format("`{}` failed to read", script.src)); continue; }
            auto err = vm -> compile_string(source, chunk_name(resource -> ref, script.src));
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
        return true;
    }

    void Resource::execute_scripts(const std::string& name, std::vector<std::pair<std::string, std::string>>& sources) {
        auto vm = Manager::Sandbox::get_singleton() -> get_vm();
        const Manifest* resource;
        {
            std::lock_guard<std::mutex> lock(mutex);
            resource = get_resource_unsafe(name);
        }
        vm -> create_environment(name);
        vm -> pop(1);
        for (const auto& [src, source] : sources) {
            vm -> get_reference(name, true);
            vm -> load_string(source, chunk_name(resource -> ref, src), true, true, vm -> get_count());
            vm -> pop(1);
        }
    }

    void Resource::execute_resource(std::string name) {
        std::vector<std::pair<std::string, std::string>> sources;
        if (!validate_scripts(name, sources)) return;

        {
            std::lock_guard<std::mutex> lock(mutex);
            running.insert(name);
        }
        log("sbox", fmt::format("resource `{}` started", name));
        execute_scripts(name, sources);
        #if !defined(Vital_SDK_Client)
            Engine::Core::get_singleton() -> push_deferred([this, name]() {
                const Manifest* resource;
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    resource = get_resource_unsafe(name);
                }
                Engine::Network::get_singleton() -> broadcast(build_packet("vital.resource:started", name, resource));
                Manager::Sandbox::get_singleton() -> signal("vital.resource:started", Tool::StackValue(name));
            });
        #else
            Manager::Sandbox::get_singleton() -> signal("vital.resource:started", Tool::StackValue(name));
        #endif
    }

    #if !defined(Vital_SDK_Client)
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
            const std::string src = Tool::YAML::get_str(node, "src");
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

    Tool::Stack Resource::build_packet(const std::string& event, const std::string& name, const Manifest* manifest) const {
        Tool::Stack msg;
        msg.object["event"] = Tool::StackValue(event);
        msg.object["name"] = Tool::StackValue(name);
        if (manifest) {
            auto packed = pack_manifest(*manifest);
            msg.object["scripts"] = std::move(packed.object["scripts"]);
            msg.object["files"] = std::move(packed.object["files"]);
        }
        return msg;
    }
    #endif


    // Facilitators //
    bool Resource::is_loaded_unsafe(const std::string& name) const {
        return std::any_of(resources.begin(), resources.end(), [&](const Manifest& r) { return r.ref == name; });
    }

    bool Resource::is_running_unsafe(const std::string& name) const {
        return running.count(name) > 0;
    }

    #if defined(Vital_SDK_Client)
    bool Resource::is_pending_unsafe(const std::string& name) const {
        return resource_assets.count(name) > 0;
    }
    #endif

    const Resource::Manifest* Resource::get_resource_unsafe(const std::string& name) const {
        for (const auto& resource : resources) {
            if (resource.ref == name) return &resource;
        }
        return nullptr;
    }

    std::vector<const Resource::Manifest*> Resource::get_all_resources_unsafe() const {
        std::vector<const Manifest*> result;
        result.reserve(resources.size());
        for (const auto& resource : resources) result.push_back(&resource);
        return result;
    }


    // Managers //
    void Resource::log(const std::string& mode, const std::string& message) const {
        Tool::print(mode, fmt::format("Resource: {}", message));
    }

    void Resource::ready() {
        static bool initialized = false;
        if (initialized) return;
        initialized = true;
        #if defined(Vital_SDK_Client)
            Tool::Event::bind("asset:file_ready", [this](Tool::Stack args) {
                if (!args.object.count("path")) return;
                const std::string path = args.object.at("path").as<std::string>();
                auto rm = Resource::get_singleton();
                std::string ready_name;
                {
                    std::lock_guard<std::mutex> lock(rm -> mutex);
                    for (auto& [name, remaining] : rm -> resource_assets) {
                        if (!remaining.count(path)) continue;
                        remaining.erase(path);
                        if (remaining.empty()) {
                            rm -> resource_assets.erase(name);
                            ready_name = name;
                        }
                        break;
                    }
                }
                if (!ready_name.empty()) {
                    log("sbox", fmt::format("resource `{}` all assets downloaded — executing scripts", ready_name));
                    rm -> start(ready_name);
                }
            });

            Tool::Event::bind("vital.network:packet", [this](Tool::Stack args) {
                if (!args.object.count("event")) return;
                const std::string event = args.object.at("event").as<std::string>();
                if (event == "vital.resource:started") {
                    if (!args.object.count("name")) return;
                    auto rm = Resource::get_singleton();
                    const std::string name = args.object.at("name").as<std::string>();
                    std::vector<Script> scripts;
                    std::vector<std::string> files;
                    unpack_manifest(args, scripts, files);
                    log("sbox", fmt::format("client received resource start: `{}`", name));
                    bool already;
                    {
                        std::lock_guard<std::mutex> lock(rm -> mutex);
                        already = rm -> is_running_unsafe(name) || rm -> is_pending_unsafe(name);
                    }
                    if (!already) rm -> load(name, scripts, files);
                }
                else if (event == "vital.resource:stopped") {
                    if (!args.object.count("name")) return;
                    auto rm = Resource::get_singleton();
                    const std::string name = args.object.at("name").as<std::string>();
                    log("sbox", fmt::format("client received resource stop: `{}`", name));
                    rm -> stop(name);
                }
            });
        #else
            scan();
            Tool::Event::bind("vital.network:peer:join", [](Tool::Stack args) {
                if (args.array.empty()) return;
                const int peer_id = args.array[0].as<int32_t>();
                Engine::Core::get_singleton() -> push_deferred([peer_id]() { Manager::Resource::get_singleton() -> sync(peer_id); });
            });
        #endif
    }

    #if !defined(Vital_SDK_Client)
    void Resource::sync(int peer_id) const {
        Manager::Asset::get_singleton() -> broadcast_manifest(peer_id);
        std::lock_guard<std::mutex> lock(mutex);
        for (const auto& name : running) {
            auto resource = get_resource_unsafe(name);
            if (!resource) continue;
            Engine::Network::get_singleton() -> send(build_packet("vital.resource:started", name, resource), peer_id);
        }
    }
    #endif


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
        std::lock_guard<std::mutex> lock(mutex);
        return is_loaded_unsafe(name);
    }

    bool Resource::is_running(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex);
        return is_running_unsafe(name);
    }

    #if defined(Vital_SDK_Client)
    bool Resource::is_pending(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex);
        return is_pending_unsafe(name);
    }
    #endif


    // Getters //
    std::vector<const Resource::Manifest*> Resource::get_all_resources() const {
        std::lock_guard<std::mutex> lock(mutex);
        return get_all_resources_unsafe();
    }

    const Resource::Manifest* Resource::get_resource(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex);
        return get_resource_unsafe(name);
    }

    std::string Resource::get_resource_from_vm(Vital::Sandbox::Machine* vm) {
        return vm -> get_environment_id();
    }

    std::string Resource::get_resource_base(const std::string& name, bool require_running) {
        if (!is_name(name)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid resource name");
        if (require_running && !get_singleton() -> is_running(name)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: resource `{}` not running", name));
        return Tool::get_directory("resources", name);
    }

    std::vector<Resource::Script> Resource::get_resource_scripts(const std::string& name, const std::string& type) const {
        std::lock_guard<std::mutex> lock(mutex);
        std::vector<Script> result;
        auto resource = get_resource_unsafe(name);
        if (!resource) return result;
        for (const auto& script : resource -> scripts) {
            if (type.empty() || script.type == type) result.push_back(script);
        }
        return result;
    }


    // APIs //
    bool Resource::start(std::string name) {
        auto am = Manager::Asset::get_singleton();
        {
            std::lock_guard<std::mutex> lock(mutex);
            #if !defined(Vital_SDK_Client)
            if (!is_loaded_unsafe(name)) { log("error", fmt::format("cannot start `{}` — resource not loaded", name)); return false; }
            #endif
            if (is_running_unsafe(name)) { log("error", fmt::format("cannot start `{}` — already running", name)); return false; }
            #if defined(Vital_SDK_Client)
            if (is_pending_unsafe(name)) { log("error", fmt::format("cannot start `{}` — already pending", name)); return false; }
            #endif
        }
        #if !defined(Vital_SDK_Client)
        {
            std::lock_guard<std::mutex> lock(mutex);
            auto resource = get_resource_unsafe(name);
            std::vector<std::string> asset_paths;
            for (const auto& file : resource -> files) asset_paths.push_back(fmt::format("resources/{}/{}", name, file));
            for (const auto& script : resource -> scripts) {
                if (script.type == "shared" || script.type == "client") asset_paths.push_back(fmt::format("resources/{}/{}", name, script.src));
            }
            am -> register_assets(asset_paths, name);
            am -> broadcast_manifest(-1, true);
        }
        #endif
        execute_resource(name);
        return true;
    }

    bool Resource::stop(std::string name) {
        auto vm = Manager::Sandbox::get_singleton() -> get_vm();
        auto am = Manager::Asset::get_singleton();
        bool was_running;
        {
            std::lock_guard<std::mutex> lock(mutex);
            #if defined(Vital_SDK_Client)
                if (!is_running_unsafe(name) && !is_pending_unsafe(name)) { log("error", fmt::format("cannot stop `{}` — not running or pending", name)); return false; }
                if (is_pending_unsafe(name)) {
                    resource_assets.erase(name);
                    am -> cancel_group(name);
                    log("sbox", fmt::format("resource `{}` download cancelled", name));
                }
            #else
                if (!is_running_unsafe(name)) { log("error", fmt::format("cannot stop `{}` — not running", name)); return false; }
                am -> unregister_group(name);
            #endif
            was_running = is_running_unsafe(name);
            if (was_running) {
                vm -> clear_environment_id(name);
                running.erase(name);
            }
            #if defined(Vital_SDK_Client)
            resources.erase(std::remove_if(resources.begin(), resources.end(), [&](const Manifest& m) { return m.ref == name; }), resources.end());
            #endif
        }
        log("sbox", fmt::format("resource `{}` stopped", name));

        #if !defined(Vital_SDK_Client)
            Engine::Core::get_singleton() -> push_deferred([this, name]() {
                Engine::Network::get_singleton() -> broadcast(build_packet("vital.resource:stopped", name));
                Manager::Sandbox::get_singleton() -> signal("vital.resource:stopped", Tool::StackValue(name));
            });
        #else
            if (was_running) Manager::Sandbox::get_singleton() -> signal("vital.resource:stopped", Tool::StackValue(name));
        #endif
        return true;
    }

    #if !defined(Vital_SDK_Client)
    void Resource::scan() {
        std::lock_guard<std::mutex> lock(mutex);
        log("sbox", "rescanning resources...");
        resources.clear();
        std::vector<std::string> contents;
        try { contents = Tool::File::contents(Tool::get_directory(), "resources", true); }
        catch (...) { log("error", "resource scan skipped — `resources/` directory not found"); return; }

        std::unordered_map<std::string, int> resource_count;
        for (const auto& path : contents) resource_count[path.substr(path.find_last_of("/\\") + 1)]++;
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
            if (!parse_manifest(resource, manifest, get_resource_base(name), errors)) {
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
        std::vector<std::string> stale;
        for (const auto& name : running) if (!is_loaded_unsafe(name)) stale.push_back(name);
        for (const auto& name : stale) log("sbox", fmt::format("resource `{}` no longer exists — stopping", name));
        if (!stale.empty()) {
            Engine::Core::get_singleton() -> push_deferred([this, stale]() {
                for (const auto& name : stale) stop(name);
            });
        }
    }

    bool Resource::restart(std::string name) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (!is_running_unsafe(name)) { log("error", fmt::format("cannot restart `{}` — not running", name)); return false; }
        }
        stop(name);
        const std::string base = get_resource_base(name);
        if (!Tool::File::exists(base, "manifest.yaml")) {
            std::lock_guard<std::mutex> lock(mutex);
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

        {
            std::lock_guard<std::mutex> lock(mutex);
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
        }
        return start(name);
    }

    void Resource::start_all() {
        log("sbox", "starting all resources...");
        int count = 0;
        std::vector<const Manifest*> all;
        {
            std::lock_guard<std::mutex> lock(mutex);
            all = get_all_resources_unsafe();
        }
        for (auto resource : all) if (start(resource -> ref)) count++;
        log("sbox", fmt::format("all resources started — {} resource(s) started", count));
    }

    void Resource::stop_all() {
        log("sbox", "stopping all resources...");
        int count = 0;
        std::unordered_set<std::string> snapshot;
        {
            std::lock_guard<std::mutex> lock(mutex);
            snapshot = running;
        }
        for (const auto& name : snapshot) if (stop(name)) count++;
        log("sbox", fmt::format("all resources stopped — {} resource(s) stopped", count));
    }

    void Resource::restart_all() {
        log("sbox", "restarting all resources...");
        int count = 0;
        std::unordered_set<std::string> snapshot;
        {
            std::lock_guard<std::mutex> lock(mutex);
            snapshot = running;
        }
        for (const auto& name : snapshot) if (restart(name)) count++;
        log("sbox", fmt::format("all resources restarted — {} resource(s) restarted", count));
    }

    #else
    bool Resource::load(std::string name, const std::vector<Script>& scripts, const std::vector<std::string>& files) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (is_running_unsafe(name) || is_pending_unsafe(name)) { log("error", fmt::format("cannot load `{}` — already running or pending", name)); return false; }
            resources.erase(std::remove_if(resources.begin(), resources.end(), [&](const Manifest& m) { return m.ref == name; }), resources.end());
            Manifest manifest;
            manifest.ref = name;
            manifest.name = name;
            manifest.scripts = scripts;
            manifest.files = files;
            resources.push_back(std::move(manifest));
        }
        log("sbox", fmt::format("resource `{}` registered from server — {} script(s), {} file(s)", name, scripts.size(), files.size()));

        std::unordered_set<std::string> asset_paths;
        {
            std::lock_guard<std::mutex> lock(mutex);
            auto resource = get_resource_unsafe(name);
            for (const auto& file : resource -> files) asset_paths.insert(fmt::format("resources/{}/{}", name, file));
            for (const auto& script : resource -> scripts) {
                if (is_type(script.type)) asset_paths.insert(fmt::format("resources/{}/{}", name, script.src));
            }
            for (const auto& path : asset_paths) {
                if (Tool::File::exists(Tool::get_directory(), path)) log("sbox", fmt::format("resource `{}` asset cached: {}", name, path));
                else resource_assets[name].insert(path);
            }
        }

        bool all_cached;
        {
            std::lock_guard<std::mutex> lock(mutex);
            all_cached = !is_pending_unsafe(name);
        }
        if (all_cached) {
            log("sbox", fmt::format("resource `{}` all assets cached — executing immediately", name));
            start(name);
        }
        else {
            std::lock_guard<std::mutex> lock(mutex);
            log("sbox", fmt::format("resource `{}` queued — {} asset(s) pending download", name, resource_assets[name].size()));
        }
        return true;
    }
    #endif
}