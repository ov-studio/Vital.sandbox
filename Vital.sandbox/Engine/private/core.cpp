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
#include <Vital.sandbox/Engine/public/model.h>
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Manager/public/asset.h>


//////////////////////////
// Vital: Engine: Core //
//////////////////////////

namespace Vital::Engine {
    // Instantiators //
    void Core::_ready() {
        singleton = singleton ? singleton : this;
        set_process(false);
        if (!Tool::is_runtime()) return;

        kit_abort.store(false);
        Tool::print("sbox", "Core: bootstrapping Vital.kit...");
        kit_thread = std::thread([this]() {
            Manager::Kit::ensure();
            if (!kit_abort.load()) {
                enqueue([this]() {
                    Tool::print("sbox", "Core: Vital.kit ready");
                    #if defined(VSDK_Client)
                    start_http_server();
                    #endif
                    kit_ready.store(true);
                    Tool::Event::emit("kit:ready");
                    Tool::Event::emit("core:ready");
                    set_process(true);
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
        Manager::Sandbox::get_singleton() -> input(event);
    }
    #endif


    // Utils //
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

    void Core::teardown() {
        Manager::Asset::free_singleton();
        Engine::Model::teardown_spawner();
        #if defined(VSDK_Client)
        stop_http_server();
        free_environment();
        #endif
        Tool::Event::emit("core:teardown");
    }

    void Core::execute(std::function<void()> exec) {
        if (Tool::is_main_thread()) exec();
        else enqueue(std::move(exec));
    }

    void Core::enqueue(std::function<void()> exec) {
        std::lock_guard<std::mutex> lock(mutex);
        work_queue.push_back(std::move(exec));
    }

    void Core::drain() {
        std::vector<std::function<void()>> local;
        {
            std::lock_guard<std::mutex> lock(mutex);
            local.swap(work_queue);
        }
        for (auto& exec : local) exec();
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

    godot::Ref<godot::Environment> Core::get_environment() {
        if (!environment) {
            environment = memnew(godot::WorldEnvironment);
            get_singleton() -> enqueue([]() {
                get_singleton() -> add_child(environment);
            });
            godot::Ref<godot::Environment> env;
            env.instantiate();
            environment -> set_environment(env);
        }
        return environment -> get_environment();
    }

    void Core::free_environment() {
        if (!environment) return;
        environment -> queue_free();
        environment = nullptr;
    }

    godot::Vector2 Core::get_resolution() {
        return get_display_server() -> window_get_size();
    }

    void Core::start_http_server() {
        if (http_running) return;

        http_server = std::make_unique<httplib::Server>();
        http_server -> set_mount_point("/cache", Tool::get_directory() + "/cache");
        http_server -> set_mount_point("/resources", Tool::get_directory() + "/resources");

        http_running = true;
        http_thread = std::thread([this]() {
            Tool::print("sbox", "Core: HTTP server starting on port ", http_port);
            http_server -> listen("127.0.0.1", http_port);
            Tool::print("sbox", "Core: HTTP server stopped");
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        Tool::print("sbox", "Core: HTTP server running on port ", http_port);
    }

    void Core::stop_http_server() {
        if (!http_running) return;
        http_running = false;
        if (http_server) http_server -> stop();
        if (http_thread.joinable()) http_thread.join();
        http_server.reset();
    }

    int Core::get_http_port() const {
        return http_port;
    }
    #endif
}