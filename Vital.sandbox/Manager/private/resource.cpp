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
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Manager/public/asset.h>
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/model.h>


///////////////////////////////
// Vital: Manager: Resource //
///////////////////////////////

namespace Vital::Manager {
    // Helpers //
    std::string Resource::Internal::chunk_name(const std::string& resource, const std::string& src) {
        return fmt::format("@{}/{}", resource, src);
    }

    Tool::Stack Resource::Internal::pack_manifest(const Manifest& manifest) {
        Tool::Stack out;
        Tool::Stack scripts;
        Tool::Stack files;
        Tool::Stack models;
        for (const auto& s : manifest.scripts) {
            Tool::Stack entry;
            entry.object["src"] = Tool::StackValue(s.src);
            entry.object["type"] = Tool::StackValue(s.type);
            scripts.array.push_back(Tool::StackValue(std::move(entry)));
        }
        out.object["scripts"] = Tool::StackValue(std::move(scripts));
        for (const auto& f : manifest.files) files.array.push_back(Tool::StackValue(f));
        out.object["files"] = Tool::StackValue(std::move(files));
        for (const auto& f : manifest.models) models.array.push_back(Tool::StackValue(f));
        out.object["models"] = Tool::StackValue(std::move(models));
        return out;
    }

    void Resource::Internal::unpack_manifest(const Tool::Stack& arguments, std::vector<Script>& scripts, std::vector<std::string>& files, std::vector<std::string>& models) {
        if (const auto* sv = arguments.get("scripts")) {
            const auto& nested = *sv -> as<std::shared_ptr<Tool::Stack>>();
            scripts.reserve(nested.array.size());
            for (const auto& entry : nested.array) {
                const auto& s = *entry.as<std::shared_ptr<Tool::Stack>>();
                scripts.push_back({ s.object.at("src").as<std::string>(), s.object.at("type").as<std::string>() });
            }
        }
        if (const auto* sv = arguments.get("files")) {
            const auto& nested = *sv -> as<std::shared_ptr<Tool::Stack>>();
            files.reserve(nested.array.size());
            for (const auto& entry : nested.array) files.push_back(entry.as<std::string>());
        }
        if (const auto* sv = arguments.get("models")) {
            const auto& nested = *sv -> as<std::shared_ptr<Tool::Stack>>();
            models.reserve(nested.array.size());
            for (const auto& entry : nested.array) models.push_back(entry.as<std::string>());
        }
    }

    bool Resource::Internal::validate_scripts(const std::string& name, std::vector<std::pair<std::string, std::string>>& sources) {
        Tool::assert_main_thread("Resource::Internal::validate_scripts");
        auto rm = Resource::get_singleton();
        auto vm = Manager::Sandbox::get_singleton() -> get_vm();
        const Manifest* resource;
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            resource = Internal::get_resource(name);
        }
        std::vector<std::string> errors;
        for (const auto& script : resource -> scripts) {
            #if defined(VSDK_Client)
            if (!Resource::is_type(script.type)) continue;
            #endif
            std::string source;
            try { source = Tool::File::read_text(Resource::get_resource_base(name), script.src); }
            catch (...) { errors.push_back(fmt::format("`{}` failed to read", script.src)); continue; }
            auto err = vm -> compile_string(source, Internal::chunk_name(resource -> ref, script.src));
            if (!err.empty()) { errors.push_back(fmt::format("`{}` failed to compile ({})", script.src, err)); continue; }
            if (Resource::is_type(script.type)) sources.emplace_back(script.src, std::move(source));
        }
        if (!errors.empty()) {
            std::string report = fmt::format("resource `{}` failed to start\n", name);
            report += fmt::format("> Errors ({}):\n", errors.size());
            for (const auto& err : errors) report += fmt::format("> {}\n", err);
            rm -> log("error", report);
            return false;
        }
        return true;
    }

    void Resource::Internal::execute_scripts(const std::string& name, std::vector<std::pair<std::string, std::string>>& sources) {
        Tool::assert_main_thread("Resource::Internal::execute_scripts");
        auto rm = Resource::get_singleton();
        auto vm = Manager::Sandbox::get_singleton() -> get_vm();
        const Manifest* resource;
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            resource = Internal::get_resource(name);
        }
        vm -> create_environment(name);
        vm -> pop(1);
        for (const auto& [src, source] : sources) {
            vm -> get_reference(vm -> get_environment_ref(name), true);
            vm -> load_string(source, Internal::chunk_name(resource -> ref, src), true, true, vm -> get_count());
            vm -> pop(1);
        }
    }

    void Resource::Internal::load_models(const std::string& name) {
        Tool::assert_main_thread("Resource::Internal::load_models");
        auto rm = Resource::get_singleton();
        const Manifest* resource;
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            resource = Internal::get_resource(name);
        }
        if (!resource) return;
    
        #if !defined(VSDK_Client)
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            const_cast<Manifest*>(resource) -> models = Engine::Model::filter_resource_models(name, resource -> files);
        }
        #endif
        if (resource -> models.empty()) return;
        Engine::Model::load_resource_models(name, resource -> models);
    }

    void Resource::Internal::execute_resource(std::string name) {
        Tool::assert_main_thread("Resource::Internal::execute_resource");
        auto rm = Resource::get_singleton();
        std::vector<std::pair<std::string, std::string>> sources;
        if (!Internal::validate_scripts(name, sources)) return;

        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            rm -> running.insert(name);
        }
        rm -> log("sbox", fmt::format("resource `{}` started", name));

        Internal::load_models(name);
        Internal::execute_scripts(name, sources);
        #if !defined(VSDK_Client)
            Engine::Core::get_singleton() -> enqueue([name]() {
                const Manifest* resource;
                {
                    auto rm = Resource::get_singleton();
                    std::lock_guard<std::mutex> lock(rm -> mutex);
                    resource = Internal::get_resource(name);
                }
                Manager::Network::get_singleton() -> broadcast(Internal::build_packet("resource:started", name, resource));
                Manager::Sandbox::get_singleton() -> signal("resource:started", Tool::StackValue(name));
            });
        #else
            Manager::Sandbox::get_singleton() -> signal("resource:started", Tool::StackValue(name));
        #endif
    }

    #if defined(VSDK_Client)
    bool Resource::Internal::register_resource(std::string name, const std::vector<Script>& scripts, const std::vector<std::string>& files, const std::vector<std::string>& models) {
        Tool::assert_main_thread("Resource::Internal::register_resource");
        auto rm = Resource::get_singleton();
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            if (Internal::is_running(name) || Internal::is_pending(name)) { rm -> log("error", fmt::format("cannot register resource `{}` — already running or pending", name)); return false; }
            rm -> resources.erase(std::remove_if(rm -> resources.begin(), rm -> resources.end(), [&](const Manifest& m) { return m.ref == name; }), rm -> resources.end());
            Manifest manifest;
            manifest.ref = name;
            manifest.name = name;
            manifest.scripts = scripts;
            manifest.files = files;
            manifest.models = models;
            rm -> resources.push_back(std::move(manifest));
        }
        rm -> log("sbox", fmt::format("resource `{}` registered from server — {} script(s), {} file(s), {} model(s)", name, scripts.size(), files.size(), models.size()));
        return start(name);
    }
    #else
    bool Resource::Internal::parse_manifest(Manifest& resource, Tool::YAML& manifest, const std::string& base, std::vector<std::string>& errors) {
        resource.name = manifest.get_str("name", resource.ref);
        resource.author = manifest.get_str("author", "");
        resource.version = manifest.get_str("version", "");
        resource.scripts.clear();
        resource.files.clear();
        resource.models.clear();
        resource.script_hashes.clear();
        resource.file_hashes.clear();
        if (!manifest.has("scripts") || !manifest.get_root()["scripts"].is_seq()) return false;

        for (ryml::ConstNodeRef node : manifest.get_root()["scripts"]) {
            if (!node.is_map() || !Tool::YAML::has(node, "src") || !Tool::YAML::has(node, "type")) { errors.push_back("script entry missing `src` or `type`"); continue; }
            const std::string src = Tool::YAML::get_str(node, "src");
            const std::string type = Tool::YAML::get_str(node, "type");
            if (!Resource::Types.count(type)) { errors.push_back(fmt::format("script `{}` has invalid type `{}`", src, type)); continue; }
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

    Tool::Stack Resource::Internal::build_packet(const std::string& event, const std::string& name, const Manifest* manifest) {
        Tool::Stack packet;
        packet.object["event"] = Tool::StackValue(event);
        packet.object["name"] = Tool::StackValue(name);
        if (manifest) {
            auto packed = Internal::pack_manifest(*manifest);
            packet.object["scripts"] = std::move(packed.object["scripts"]);
            packet.object["files"] = std::move(packed.object["files"]);
            packet.object["models"] = std::move(packed.object["models"]);
        }
        return packet;
    }
    #endif


    // Checkers //
    bool Resource::Internal::is_loaded(const std::string& name) {
        auto rm = Resource::get_singleton();
        return std::any_of(rm -> resources.begin(), rm -> resources.end(), [&](const Manifest& r) { return r.ref == name; });
    }

    bool Resource::Internal::is_running(const std::string& name) {
        auto rm = Resource::get_singleton();
        return rm -> running.count(name) > 0;
    }

    #if defined(VSDK_Client)
    bool Resource::Internal::is_pending(const std::string& name) {
        auto rm = Resource::get_singleton();
        return rm -> resource_assets.count(name) > 0;
    }
    #endif


    // Getters //
    const Resource::Manifest* Resource::Internal::get_resource(const std::string& name) {
        auto rm = Resource::get_singleton();
        for (const auto& resource : rm -> resources) {
            if (resource.ref == name) return &resource;
        }
        return nullptr;
    }

    std::vector<const Resource::Manifest*> Resource::Internal::get_all_resources() {
        auto rm = Resource::get_singleton();
        std::vector<const Manifest*> result;
        result.reserve(rm -> resources.size());
        for (const auto& resource : rm -> resources) result.push_back(&resource);
        return result;
    }

    std::vector<const Resource::Manifest*> Resource::Internal::get_resources(State type) {
        auto rm = Resource::get_singleton();
        std::vector<const Manifest*> result;
        switch (type) {
            case State::Loaded:
                result.reserve(rm -> resources.size());
                for (const auto& r : rm -> resources) result.push_back(&r);
                break;
            case State::Running:
                result.reserve(rm -> running.size());
                for (const auto& name : rm -> running) result.push_back(get_resource(name));
                break;
        }
        return result;
    }


    // APIs //
    bool Resource::Internal::start(std::string name) {
        Tool::assert_main_thread("Resource::Internal::start");
        auto rm = Resource::get_singleton();
        auto am = Manager::Asset::get_singleton();
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            #if !defined(VSDK_Client)
            if (!Internal::is_loaded(name)) { rm -> log("error", fmt::format("cannot start `{}` — resource not loaded", name)); return false; }
            #endif
            if (Internal::is_running(name)) { rm -> log("error", fmt::format("cannot start `{}` — already running", name)); return false; }
            #if defined(VSDK_Client)
            if (Internal::is_pending(name)) { rm -> log("error", fmt::format("cannot start `{}` — already pending", name)); return false; }
            #endif
        }
        #if !defined(VSDK_Client)
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            auto resource = Internal::get_resource(name);
            std::vector<std::string> asset_paths;
            for (const auto& file : resource -> files) asset_paths.push_back(fmt::format("resources/{}/{}", name, file));
            for (const auto& script : resource -> scripts) {
                if (script.type == "shared" || script.type == "client") asset_paths.push_back(fmt::format("resources/{}/{}", name, script.src));
            }
            am -> register_assets(asset_paths, name);
            am -> broadcast_manifest(-1, true);
        }
        #else
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            auto resource = Internal::get_resource(name);
            std::unordered_set<std::string> asset_paths;
            for (const auto& file : resource -> files) asset_paths.insert(fmt::format("resources/{}/{}", name, file));
            for (const auto& script : resource -> scripts) {
                if (is_type(script.type)) asset_paths.insert(fmt::format("resources/{}/{}", name, script.src));
            }
            for (const auto& path : asset_paths) {
                if (Tool::File::exists(Tool::get_directory(), path)) rm -> log("sbox", fmt::format("resource `{}` asset cached: {}", name, path));
                else rm -> resource_assets[name].insert(path);
            }
            if (Internal::is_pending(name)) {
                rm -> log("sbox", fmt::format("resource `{}` queued — {} asset(s) pending download", name, rm -> resource_assets[name].size()));
                return true;
            }
        }
        #endif
        Internal::execute_resource(name);
        return true;
    }

    bool Resource::Internal::stop(std::string name) {
        Tool::assert_main_thread("Resource::Internal::stop");
        auto rm = Resource::get_singleton();
        auto vm = Manager::Sandbox::get_singleton() -> get_vm();
        auto am = Manager::Asset::get_singleton();
        bool was_running;
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            #if defined(VSDK_Client)
                if (!Internal::is_running(name) && !Internal::is_pending(name)) { rm -> log("error", fmt::format("cannot stop `{}` — not running or pending", name)); return false; }
                if (Internal::is_pending(name)) {
                    rm -> resource_assets.erase(name);
                    am -> cancel_group(name);
                    rm -> log("sbox", fmt::format("resource `{}` download cancelled", name));
                }
            #else
                if (!Internal::is_running(name)) { rm -> log("error", fmt::format("cannot stop `{}` — not running", name)); return false; }
                am -> unregister_group(name);
            #endif
            was_running = Internal::is_running(name);
            if (was_running) rm -> running.erase(name);
            #if defined(VSDK_Client)
            rm -> resources.erase(std::remove_if(rm -> resources.begin(), rm -> resources.end(), [&](const Manifest& m) { return m.ref == name; }), rm -> resources.end());
            #endif
        }
        if (was_running) {
            vm -> clear_environment_id(name);
            Engine::Model::unload_resource_models(name);
            Manager::Sandbox::get_singleton() -> reset_exports(name);
        }
        rm -> log("sbox", fmt::format("resource `{}` stopped", name));

        #if !defined(VSDK_Client)
            Engine::Core::get_singleton() -> enqueue([rm, name]() {
                Manager::Network::get_singleton() -> broadcast(Internal::build_packet("resource:stopped", name));
                Manager::Sandbox::get_singleton() -> signal("resource:stopped", Tool::StackValue(name));
            });
        #else
            if (was_running) Manager::Sandbox::get_singleton() -> signal("resource:stopped", Tool::StackValue(name));
        #endif
        return true;
    }

    void Resource::Internal::stop_all() {
        Tool::assert_main_thread("Resource::Internal::stop_all");
        auto rm = Resource::get_singleton();
        rm -> log("sbox", "stopping all resources...");
        int count = 0;
        std::unordered_set<std::string> snapshot;
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            snapshot = rm -> running;
        }
        for (const auto& name : snapshot) if (stop(name)) count++;
        rm -> log("sbox", fmt::format("all resources stopped — {} resource(s) stopped", count));
    }

    #if !defined(VSDK_Client)
    void Resource::Internal::scan() {
        Tool::assert_main_thread("Resource::Internal::scan");
        auto rm = Resource::get_singleton();
        std::lock_guard<std::mutex> lock(rm -> mutex);
        rm -> log("sbox", "rescanning resources...");
        rm -> resources.clear();
        std::vector<std::string> contents;
        try { contents = Tool::File::contents(Tool::get_directory(), "resources", true); }
        catch (...) { rm -> log("error", "resource scan skipped — `resources/` directory not found"); return; }

        std::unordered_map<std::string, int> resource_count;
        for (const auto& path : contents) resource_count[path.substr(path.find_last_of("/\\") + 1)]++;
        auto try_read = [](const std::string& base, const std::string& file, std::string& out) -> bool {
            try { out = Tool::File::read_text(base, file); return true; }
            catch (...) { return false; }
        };

        for (const auto& path : contents) {
            const std::string name = path.substr(path.find_last_of("/\\") + 1);
            if (!Resource::is_name(name)) { rm -> log("error", fmt::format("invalid resource name `{}` — skipping", name)); continue; }
            if (resource_count[name] > 1) { rm -> log("error", fmt::format("duplicate resource found — skipping `{}`", name)); continue; }
            if (!Tool::File::exists(Resource::get_resource_base(name), "manifest.yaml")) continue;

            std::string content;
            if (!try_read(Resource::get_resource_base(name), "manifest.yaml", content)) { rm -> log("error", fmt::format("failed to read manifest for `{}` — skipping", name)); continue; }
            Tool::YAML manifest;
            try { manifest.parse(content); }
            catch (const std::exception& e) { rm -> log("error", fmt::format("malformed yaml in manifest for `{}` — skipping\n> {}", name, e.what())); continue; }
            Manifest resource;
            resource.ref = name;
            std::vector<std::string> errors;
            if (!Internal::parse_manifest(resource, manifest, Resource::get_resource_base(name), errors)) {
                if (!errors.empty()) {
                    std::string report = fmt::format("resource `{}` skipped:\n", name);
                    report += fmt::format("> Errors ({}):\n", errors.size());
                    for (const auto& err : errors) report += fmt::format("> {}\n", err);
                    rm -> log("error", report);
                }
                else rm -> log("error", fmt::format("resource `{}` has no valid `scripts` section — skipping", name));
                continue;
            }
            rm -> resources.push_back(std::move(resource));
        }

        std::string report = fmt::format("scan complete — {} resource(s) loaded\n", rm -> resources.size());
        for (const auto& resource : rm -> resources) report += fmt::format("> `{}`\n", resource.ref);
        rm -> log("sbox", report);
        std::vector<std::string> stale;
        for (const auto& name : rm -> running) if (!Internal::is_loaded(name)) stale.push_back(name);
        for (const auto& name : stale) rm -> log("sbox", fmt::format("resource `{}` no longer exists — stopping", name));
        if (!stale.empty()) {
            Engine::Core::get_singleton() -> enqueue([stale]() {
                for (const auto& name : stale) Internal::stop(name);
            });
        }
    }

    bool Resource::Internal::restart(std::string name) {
        Tool::assert_main_thread("Resource::Internal::restart");
        auto rm = Resource::get_singleton();
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            if (!Internal::is_running(name)) { rm -> log("error", fmt::format("cannot restart `{}` — not running", name)); return false; }
        }

        const std::string base = Resource::get_resource_base(name);
        if (!Tool::File::exists(base, "manifest.yaml")) {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            rm -> resources.erase(std::remove_if(rm -> resources.begin(), rm -> resources.end(), [&](const Manifest& m) { return m.ref == name; }), rm -> resources.end());
            rm -> log("error", fmt::format("resource `{}` manifest not found — unregistered", name));
            return false;
        }
        std::string content;
        try { content = Tool::File::read_text(base, "manifest.yaml"); }
        catch (...) { rm -> log("error", fmt::format("resource `{}` failed to read manifest — skipping restart", name)); return false; }
        Tool::YAML manifest;
        try { manifest.parse(content); }
        catch (const std::exception& e) { rm -> log("error", fmt::format("resource `{}` malformed yaml — {} — skipping restart", name, e.what())); return false; }

        std::string change_report;
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            for (auto& resource : rm -> resources) {
                if (resource.ref != name) continue;
                const auto old_script_hashes = resource.script_hashes;
                const auto old_file_hashes = resource.file_hashes;
                std::vector<std::string> errors;
                if (!Internal::parse_manifest(resource, manifest, base, errors)) {
                    if (!errors.empty()) {
                        std::string report = fmt::format("resource `{}` restart aborted\n", name);
                        report += fmt::format("> Errors ({}):\n", errors.size());
                        for (const auto& err : errors) report += fmt::format("> {}\n", err);
                        rm -> log("error", report);
                    }
                    else rm -> log("error", fmt::format("resource `{}` has no valid `scripts` section — skipping restart", name));
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
                change_report = report;
                break;
            }
        }
        rm -> log("sbox", change_report);
        stop(name);
        Engine::Core::get_singleton() -> enqueue([name]() { Internal::start(name); });
        return true;
    }

    void Resource::Internal::start_all() {
        Tool::assert_main_thread("Resource::Internal::start_all");
        auto rm = Resource::get_singleton();
        rm -> log("sbox", "starting all resources...");
        int count = 0;
        std::vector<const Manifest*> all;
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            all = Internal::get_all_resources();
        }
        for (auto resource : all) if (start(resource -> ref)) count++;
        rm -> log("sbox", fmt::format("all resources started — {} resource(s) started", count));
    }

    void Resource::Internal::restart_all() {
        Tool::assert_main_thread("Resource::Internal::restart_all");
        auto rm = Resource::get_singleton();
        rm -> log("sbox", "restarting all resources...");
        int count = 0;
        std::unordered_set<std::string> snapshot;
        {
            std::lock_guard<std::mutex> lock(rm -> mutex);
            snapshot = rm -> running;
        }
        for (const auto& name : snapshot) if (restart(name)) count++;
        rm -> log("sbox", fmt::format("all resources restarted — {} resource(s) restarted", count));
    }
    #endif
}

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


    // Managers //
    void Resource::log(const std::string& mode, const std::string& message) const {
        Tool::print(mode, fmt::format("Resource: {}", message));
    }

    void Resource::ready() {
        static bool initialized = false;
        if (initialized) return;
        initialized = true;
        #if defined(VSDK_Client)
            Tool::Event::bind("asset:file_ready", [this](Tool::Stack arguments) {
                if (!arguments.object.count("path")) return;
                const std::string path = arguments.object.at("path").as<std::string>();
                auto rm = Resource::get_singleton();
                std::string ready_name;
                {
                    std::lock_guard<std::mutex> lock(rm -> mutex);
                    for (auto& [name, remaining] : rm -> resource_assets) {
                        if (!remaining.count(path)) continue;
                        remaining.erase(path);
                        if (remaining.empty()) ready_name = name;
                        break;
                    }
                    if (!ready_name.empty()) rm -> resource_assets.erase(ready_name);
                }
                if (!ready_name.empty()) {
                    log("sbox", fmt::format("resource `{}` all assets downloaded — executing scripts", ready_name));
                    Internal::start(ready_name);
                }
            });

            Tool::Event::bind("network:packet", [this](Tool::Stack arguments) {
                if (!arguments.object.count("event")) return;
                const std::string event = arguments.object.at("event").as<std::string>();
                if (event == "resource:started") {
                    if (!arguments.object.count("name")) return;
                    auto rm = Resource::get_singleton();
                    const std::string name = arguments.object.at("name").as<std::string>();
                    std::vector<Script> scripts;
                    std::vector<std::string> files;
                    std::vector<std::string> models;
                    Internal::unpack_manifest(arguments, scripts, files, models);
                    log("sbox", fmt::format("client received resource start: `{}`", name));
                    bool already;
                    {
                        std::lock_guard<std::mutex> lock(rm -> mutex);
                        already = Internal::is_running(name) || Internal::is_pending(name);
                    }
                    if (!already) Engine::Core::get_singleton() -> enqueue([name, scripts, files, models]() { Internal::register_resource(name, scripts, files, models); });
                }
                else if (event == "resource:stopped") {
                    if (!arguments.object.count("name")) return;
                    auto rm = Resource::get_singleton();
                    const std::string name = arguments.object.at("name").as<std::string>();
                    log("sbox", fmt::format("client received resource stop: `{}`", name));
                    Internal::stop(name);
                }
            });
        #else
            scan();
            Tool::Event::bind("network:peer:join", [](Tool::Stack arguments) {
                if (arguments.array.empty()) return;
                const int peer_id = arguments.array[0].as<int32_t>();
                Engine::Core::get_singleton() -> enqueue([peer_id]() { Manager::Resource::get_singleton() -> sync(peer_id); });
            });
        #endif
    }

    #if !defined(VSDK_Client)
    void Resource::sync(int peer_id) const {
        Manager::Asset::get_singleton() -> broadcast_manifest(peer_id);
        std::lock_guard<std::mutex> lock(mutex);
        for (const auto& name : running) {
            auto resource = Internal::get_resource(name);
            if (!resource) continue;
            Manager::Network::get_singleton() -> send(Internal::build_packet("resource:started", name, resource), peer_id);
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
        return Internal::is_loaded(name);
    }

    bool Resource::is_running(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex);
        return Internal::is_running(name);
    }

    #if defined(VSDK_Client)
    bool Resource::is_pending(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex);
        return Internal::is_pending(name);
    }
    #endif


    // Getters //
    std::vector<const Resource::Manifest*> Resource::get_all_resources() const {
        std::lock_guard<std::mutex> lock(mutex);
        return Internal::get_all_resources();
    }

    const Resource::Manifest* Resource::get_resource(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex);
        return Internal::get_resource(name);
    }

    std::string Resource::get_resource_from_vm(Vital::Sandbox::Machine* vm) {
        Tool::assert_main_thread("Resource::get_resource_from_vm");
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
        auto resource = Internal::get_resource(name);
        if (!resource) return result;
        for (const auto& script : resource -> scripts) {
            if (type.empty() || script.type == type) result.push_back(script);
        }
        return result;
    }

    std::vector<const Resource::Manifest*> Resource::get_resources(State type) const {
        std::lock_guard<std::mutex> lock(mutex);
        return Internal::get_resources(type);
    }

    int Resource::get_resource_count(State type) const {
        std::lock_guard<std::mutex> lock(mutex);
        return static_cast<int>(Internal::get_resources(type).size());
    }


    // APIs //
    bool Resource::start(std::string name) {
        Engine::Core::get_singleton() -> enqueue([name]() { Internal::start(name); });
        return true;
    }

    bool Resource::stop(std::string name) {
        Engine::Core::get_singleton() -> enqueue([name]() { Internal::stop(name); });
        return true;
    }

    void Resource::stop_all() {
        Engine::Core::get_singleton() -> enqueue([]() { Internal::stop_all(); });
    }

    #if !defined(VSDK_Client)
    void Resource::scan() {
        Engine::Core::get_singleton() -> enqueue([]() { Internal::scan(); });
    }

    bool Resource::restart(std::string name) {
        Engine::Core::get_singleton() -> enqueue([name]() { Internal::restart(name); });
        return true;
    }

    void Resource::start_all() {
        Engine::Core::get_singleton() -> enqueue([]() { Internal::start_all(); });
    }

    void Resource::restart_all() {
        Engine::Core::get_singleton() -> enqueue([]() { Internal::restart_all(); });
    }
    #endif
}