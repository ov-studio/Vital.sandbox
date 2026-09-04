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


//////////////////////////
// Vital: Engine: Core //
//////////////////////////

namespace Vital::Engine {
    // Hooks //
    void Core::_ready() {
        singleton = singleton ? singleton : this;
        set_process(false);
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
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (!work_queue.empty()) call_deferred("drain");
        }
        Manager::Sandbox::get_singleton() -> process(delta);
    }

    #if defined(VSDK_Client)
    void Core::_unhandled_input(godot::Ref<godot::InputEvent> event) {
        if (!is_ready()) return;
        // TODO: REMOVE LATER
        Tool::print("warn", "dada 2");
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

    // TODO: Improve
    void Core::when_parent_ready(godot::Node3D* node, godot::Node* target,
                                  std::function<void(godot::Node3D*, godot::Node*)> fn) {
        if (!node) return;
        if (node -> is_inside_tree() && (!target || target -> is_inside_tree())) {
            fn(node, target);
            return;
        }
        godot::ObjectID node_id   = godot::ObjectID(node -> get_instance_id());
        godot::ObjectID target_id = target ? godot::ObjectID(target -> get_instance_id()) : godot::ObjectID();
        enqueue([node_id, target_id, fn]() {
            auto* n = godot::Object::cast_to<godot::Node3D>(godot::ObjectDB::get_instance(node_id));
            if (!n) return; // node was destroyed before this ran
            godot::Node* t = target_id.is_valid()
                ? godot::Object::cast_to<godot::Node>(godot::ObjectDB::get_instance(target_id))
                : nullptr;
            if (target_id.is_valid() && !t) return; // requested target was destroyed
            // Either side may still not be ready (e.g. both created the same
            // tick) — re-enter through the same guard rather than assuming
            // one deferral is always enough.
            auto* core = Core::get_singleton();
            if (core) core -> when_parent_ready(n, t, fn);
        });
    }

    void Core::drain() {
        std::vector<std::function<void()>> local;
        {
            std::lock_guard<std::mutex> lock(mutex);
            local.swap(work_queue);
        }
        for (auto& exec : local) exec();
    }

    void Core::teardown() {
        Manager::Asset::free_singleton();
        Engine::Model::teardown_spawner();
        #if defined(VSDK_Client)
        http_server.stop();
        free_environment();
        #endif
        Tool::Event::emit("core:teardown");
    }

    void Core::shutdown() {
        Tool::print("sbox", "Core: shutting down...");
        Manager::Resource::get_singleton() -> stop_all();
        enqueue([this]() {
            Tool::print("sbox", "Core: shut down successfully!");
            // TODO: Use some hook? maybe teardown needs to be called here? but then asset etc should be torn down when disconnect instead of just shutdown...
            #if !defined(VSDK_Client)
            Manager::Masterlist::get_singleton() -> stop();
            #endif
            Engine::Console::get_singleton() -> teardown();
            std::this_thread::sleep_for(std::chrono::milliseconds(2500));
            free_singleton();
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