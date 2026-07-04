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
#include <Vital.sandbox/Manager/public/kit.h>
#include <Vital.sandbox/Manager/public/asset.h>
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Manager/public/resource.h>


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
                    http_server.add_mount("/cache", Tool::get_directory() + "/cache");
                    http_server.add_mount("/resources", Tool::get_directory() + "/resources");
                    http_server.start(true);
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

    std::string Core::get_http_url(const std::string& path) const {
        return http_server.get_url(path);
    }
    #endif
}