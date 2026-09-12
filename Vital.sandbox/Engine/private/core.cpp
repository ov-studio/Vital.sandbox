/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: core.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Core Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/console.h>
#include <Vital.sandbox/Engine/public/splash.h>
#include <Vital.sandbox/Engine/public/model.h>
#include <Vital.sandbox/Manager/public/kit.h>
#include <Vital.sandbox/Manager/public/asset.h>
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Manager/public/resource.h>
#include <Vital.sandbox/Manager/public/masterlist.h>
#include <Vital.sandbox/Manager/public/network.h>


//////////////////////////
// Vital: Engine: Core //
//////////////////////////

namespace Vital::Engine {
    // Hooks //
    void Core::_ready() {
        singleton = singleton ? singleton : this;
        set_process(false);
        set_physics_process(false);
        if (!Tool::is_runtime()) return;

        kit_abort.store(false);
        Tool::Event::emit("core:preready");
        Tool::print("sbox", "Core: bootstrapping Vital.kit...");
        kit_thread = std::thread([this]() {
            Manager::Kit::ensure();
            if (!kit_abort.load()) {
                enqueue([this]() {
                    Tool::print("sbox", "Core: Vital.kit ready");
                    #if defined(VSDK_Client)
                    http_server.set_bind_address("127.0.0.1");
                    http_server.set_label("Core");
                    http_server.add_mount("/cache", Tool::get_directory("cache"));
                    http_server.add_mount("/resources", Tool::get_directory("resources"));
                    http_server.start(true);
                    #endif
                    kit_ready.store(true);
                    Tool::Event::emit("kit:ready");
                    Tool::Event::emit("core:ready");
                    set_process(true);
                    set_physics_process(true);
                    #if !defined(VSDK_Client)
                    set_process_unhandled_input(true);
                    #endif
                });
                call_deferred("drain");
            }
        });
    }

    void Core::_exit_tree() {
        kit_abort.store(true);
        kit_ready.store(false);
        if (kit_thread.joinable()) kit_thread.join();
        {
            std::lock_guard<std::mutex> lock(mutex);
            work_queue.clear();
        }
        if (!is_ready()) return;
        teardown();
        Tool::Event::emit("core:free");
    }

    void Core::_process(double delta) {
        if (!is_ready()) return;
        Manager::Sandbox::get_singleton() -> drain();
        Manager::Sandbox::get_singleton() -> process(delta);
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (!work_queue.empty()) call_deferred("drain");
        }
    }

    // Runs once per fixed physics tick — the same clock that actually moves
    // RigidBody3D/CharacterBody3D transforms (set_physics_ticks_per_second()
    // in Network::host()). Sampling and broadcasting authoritative sync
    // transforms here (instead of from the variable-rate _process above)
    // means every outgoing snapshot corresponds to exactly one simulation
    // step: no more repeated samples of a transform that hasn't moved yet,
    // no more single-frame catch-up spikes once physics finally steps. See
    // Manager::Network::sync_tick() for the sending logic itself.
    void Core::_physics_process(double delta) {
        if (!is_ready()) return;
        Manager::Network::get_singleton() -> sync_tick(delta);
        // TODO: wIRE physics_process signal to sanhdbox
    }

    #if defined(VSDK_Client)
    void Core::_unhandled_input(godot::Ref<godot::InputEvent> event) {
        if (!is_ready()) return;
        Manager::Sandbox::get_singleton() -> input(event);
    }
    #endif


    // Singleton //
    Core* Core::get_singleton() {
        return singleton;
    }

    void Core::free_singleton() {
        if (!singleton) return;
        singleton -> get_scene_tree() -> quit(0);
    }


    // Managers //
    bool Core::is_ready() {
        return Tool::is_runtime() && kit_ready.load();
    }

    #if defined(VSDK_Client)
    bool Core::is_sandbox_ui_ready() {
        static const std::vector<std::function<bool()>> checks = {
            [] { return Console::has_singleton() && Console::get_singleton() -> is_ready(); },
            [] { return Splash::has_singleton() && Splash::get_singleton() -> is_ready(); }
        };
        return std::any_of(checks.begin(), checks.end(), [](auto& check) { return check(); });
    }

    bool Core::is_sandbox_ui_visible() {
        static const std::vector<std::function<bool()>> checks = {
            [] { return Console::has_singleton() && Console::get_singleton() -> is_visible(); },
            [] { return Splash::has_singleton() && Splash::get_singleton() -> is_visible(); }
        };
        return std::any_of(checks.begin(), checks.end(), [](auto& check) { return check(); });
    }
    #endif

    void Core::execute(std::function<void()> exec) {
        if (Tool::is_main_thread()) exec();
        else enqueue(std::move(exec));
    }

    void Core::enqueue(std::function<void()> exec) {
        std::lock_guard<std::mutex> lock(mutex);
        work_queue.push_back(std::move(exec));
    }

    void Core::execute_when_ready(godot::Node3D* node, godot::Node* target, std::function<void(godot::Node3D*, godot::Node*)> exec) {
        if (!node) return;
        if (node -> is_inside_tree() && (!target || target -> is_inside_tree())) {
            exec(node, target);
            return;
        }
        godot::ObjectID node_id = godot::ObjectID(node -> get_instance_id());
        godot::ObjectID target_id = target ? godot::ObjectID(target -> get_instance_id()) : godot::ObjectID();
        enqueue([node_id, target_id, exec]() {
            auto __node = godot::Object::cast_to<godot::Node3D>(godot::ObjectDB::get_instance(node_id));
            if (!__node) return;
            godot::Node* __target = target_id.is_valid() ? godot::Object::cast_to<godot::Node>(godot::ObjectDB::get_instance(target_id)) : nullptr;
            if (target_id.is_valid() && !__target) return;
            auto core = Core::get_singleton();
            if (core) core -> execute_when_ready(__node, __target, exec);
        });
    }
    
    #if defined(VSDK_Benchmark)
    void Core::emit_native(const std::string& name, const Tool::Stack& payload) {
        execute([this, name, payload]() {
            emit_signal("native_event", Tool::to_godot_string(name), payload.to_dict());
        });
    }
    #endif

    void Core::drain() {
        std::vector<std::function<void()>> local;
        {
            std::lock_guard<std::mutex> lock(mutex);
            local.swap(work_queue);
        }
        for (auto& exec : local) exec();
    }
    
    void Core::teardown() {
        Manager::Resource::get_singleton() -> stop_all();
        #if !defined(VSDK_Client)
        Manager::Masterlist::free_singleton();
        #endif
        Manager::Network::free_singleton();
        Manager::Asset::free_singleton();
        Engine::Model::teardown_spawner();
        #if defined(VSDK_Client)
        free_environment();
        #endif
        Tool::Event::emit("core:teardown");
    }

    void Core::shutdown() {
        Tool::print("sbox", "Core: shutting down...");
        enqueue([this]() {
            Tool::print("sbox", "Core: shut down successfully!");
            Engine::Console::get_singleton() -> teardown();
            std::this_thread::sleep_for(std::chrono::milliseconds(2500));
            free_singleton();
        });
    }

    // TODO: Improve
    void Core::session_end() {
        Tool::print("sbox", "Core: ending session...");

        // Stops every running resource. Each one's own Internal::stop() already
        // clears its Lua environment out of the (still-alive) VM and unloads its
        // cached model assets — see Manager::Resource::Internal::stop(). This call
        // enqueues its real work; see the enqueue() below for why that matters.
        Manager::Resource::get_singleton() -> stop_all();

        #if defined(VSDK_Client)
        // Resource::stop() only unloads a resource's *cached* model assets
        // (Model::unload_resource_models — the PackedScene templates), not any
        // already-spawned instance. destroy_all_syncables() below covers every
        // live networked entity regardless of authority, but a Model spawned
        // purely locally (never synced) wouldn't be in that registry at all, so
        // cleanup_spawned() sweeps every Model child of Core as a second pass —
        // same belt-and-suspenders pairing already used by the disconnect path
        // this replaces (see event.cpp's old "network:server:disconnect" bind).
        Manager::Network::get_singleton() -> destroy_all_syncables();
        Engine::Model::cleanup_spawned();
        Manager::Asset::get_singleton() -> clear();
        free_environment();
        #endif

        // Queued after stop_all()'s own enqueue() above, on the same work_queue —
        // drain() runs everything currently queued in one FIFO pass, so this is
        // guaranteed to execute only once every resource's stop handling (which
        // still needs a live VM to run its Lua-side "resource:stopped" reaction)
        // has fully finished. Freeing the Sandbox singleton here — rather than
        // just clearing its exports/state — means the *next* get_singleton() call
        // (whenever the next server connection starts a resource) constructs a
        // brand new Vital::Sandbox::Machine: no leftover globals, no leftover
        // registered event handlers, nothing at all surviving from this session.
        enqueue([]() {
            Manager::Sandbox::free_singleton();
            Tool::Event::emit("core:session:end");
        });
    }


    // Misc //
    godot::SceneTree* Core::get_scene_tree() {
        return godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton() -> get_main_loop());
    }

    godot::Window* Core::get_scene_root() {
        return get_scene_tree() -> get_root();
    }

    #if defined(VSDK_Client)
    godot::DisplayServer* Core::get_display_server() {
        return godot::DisplayServer::get_singleton();
    }

    godot::RenderingServer* Core::get_rendering_server() {
        return godot::RenderingServer::get_singleton();
    }

    godot::AudioServer* Core::get_audio_server() {
        return godot::AudioServer::get_singleton();
    }

    godot::Ref<godot::Environment> Core::get_environment() {
        if (!environment) {
            environment = memnew(godot::WorldEnvironment);
            get_singleton() -> enqueue([]() {
                get_singleton() -> add_child(environment);
            });
            godot::Ref<godot::Environment> env;
            env.instantiate();
            environment -> set_environment(env);
            get_sky();
            Tool::Event::emit("environment:ready");
        }
        return environment -> get_environment();
    }

    godot::Ref<godot::Sky> Core::get_sky() {
        auto environment = get_environment();
        godot::Ref<godot::Sky> sky = environment -> get_sky();
        if (!sky.is_valid()) {
            sky.instantiate();
            environment -> set_sky(sky);
        }
        return sky;
    }

    void Core::free_environment() {
        if (!environment) return;
        environment -> queue_free();
        environment = nullptr;
        Tool::Event::emit("environment:free");
    }

    void Core::reset_environment() {
        free_environment();
        get_environment();
    }

    godot::Vector2 Core::get_resolution() {
        return get_display_server() -> window_get_size();
    }

    std::string Core::get_http_url(const std::string& path) const {
        return http_server.get_url(path);
    }

    void Core::capture_screenshot(const std::string& base, const std::string& path) {
        auto target = Tool::to_godot_string(base + "/" + path);
        auto image = get_scene_root() -> get_texture() -> get_image();
        godot::DirAccess::make_dir_recursive_absolute(target.get_base_dir());
        if (!image.is_valid()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "failed to capture screenshot");
        if (image -> save_png(target) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("failed to save screenshot"));
    }
    #endif
}
