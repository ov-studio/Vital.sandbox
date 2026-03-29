/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: core.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Core Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/core.h>
#include <Vital.extension/Engine/public/console.h>
#include <Vital.extension/Engine/public/network.h>
#include <Vital.extension/Engine/public/asset.h>
#include <Vital.extension/Engine/public/model.h>
#include <Vital.extension/Engine/public/canvas.h>
#include <Vital.extension/Engine/public/resource.h>
#include <Vital.extension/Sandbox/index.h>


////////////////////////////
// Vital: Engine: Engine //
////////////////////////////

namespace Vital::Engine {
    // TODO: Improve
    // Instantiators //
    void Core::_ready() {
        singleton = singleton ? singleton : this;
        set_process(false);
        if (!Vital::is_runtime()) return;

        kit_abort.store(false);
        Vital::print("sbox", "Core: bootstrapping Vital.kit...");
        kit_thread = std::thread([this]() {
            Vital::Tool::Kit::ensure_kit();
            if (!kit_abort.load()) {
                push_deferred([this]() {
                    Vital::print("sbox", "Core: Vital.kit ready — firing vital.core:ready");
                    Vital::Tool::Event::emit("vital.kit:ready");
                    Vital::Tool::Event::emit("vital.core:ready");
                    set_process(true);
                });
                call_deferred("flush_deferred_queue");
            }
        });
    }

    void Core::push_deferred(std::function<void()> fn) {
        std::lock_guard<std::mutex> lock(deferred_mutex);
        deferred_queue.push_back(std::move(fn));
    }

    void Core::flush_deferred_queue() {
        std::vector<std::function<void()>> local;
        {
            std::lock_guard<std::mutex> lock(deferred_mutex);
            local.swap(deferred_queue);
        }
        for (auto& fn : local) fn();
    }

    void Core::_exit_tree() {
        kit_abort.store(true);
        if (kit_thread.joinable()) kit_thread.join();
        {
            std::lock_guard<std::mutex> lock(deferred_mutex);
            deferred_queue.clear();
        }
        #if defined(Vital_SDK_Client)
        free_environment();
        #endif
        if (!Vital::is_runtime()) return;
        teardown_singleton();
        Vital::Tool::Event::emit("vital.core:free");
    }

    void Core::_process(double delta) {
        if (!Vital::is_runtime()) return;
        {
            std::lock_guard<std::mutex> lock(deferred_mutex);
            if (!deferred_queue.empty()) call_deferred("flush_deferred_queue");
        }
        Sandbox::get_singleton() -> process(delta);
    }

    #if defined(Vital_SDK_Client)
    void Core::_unhandled_input(godot::Ref<godot::InputEvent> event) {
        if (!Vital::is_runtime()) return;
        Sandbox::get_singleton() -> input(event);
    }
    #endif


    // Utils //
    Core* Core::get_singleton() {
        return singleton;
    }

    void Core::free_singleton() {
        get_scene_tree() -> quit(0);
    }

    void Core::teardown_singleton() {
        AssetManager::free_singleton();
        Model::teardown_spawner();
    }


    // APIs //
    godot::SceneTree* Core::get_scene_tree() {
        return godot::Object::cast_to<godot::SceneTree>(
            godot::Engine::get_singleton() -> get_main_loop()
        );
    }

    godot::Window* Core::get_scene_root() {
        return get_scene_tree() -> get_root();
    }

    #if defined(Vital_SDK_Client)
    godot::DisplayServer* Core::get_display_server() {
        return godot::DisplayServer::get_singleton();
    }

    godot::RenderingServer* Core::get_rendering_server() {
        return godot::RenderingServer::get_singleton();
    }

    godot::Ref<godot::Environment> Core::get_environment() {
        if (!environment) {
            environment = memnew(godot::WorldEnvironment);
            get_singleton() -> push_deferred([]() {
                get_singleton() -> add_child(environment);
            });
            godot::Ref<godot::Environment> env;
            env.instantiate();
            environment->set_environment(env);
        }
        return environment->get_environment();
    }

    void Core::free_environment() {
        if (!environment) return;
        environment->queue_free();
        environment = nullptr;
    }
    #endif
}