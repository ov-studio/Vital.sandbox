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
        if (!Vital::is_runtime()) return;

        kit_abort.store(false);
        Vital::print("sbox", "Core: bootstrapping Vital.kit...");
        kit_thread = std::thread([this]() {
            Vital::Tool::Kit::ensure_kit();
            if (!kit_abort.load()) {
                push_deferred([this]() {
                    Vital::print("sbox", "Core: Vital.kit ready");
                    kit_ready.store(true);
                    Vital::Tool::Event::emit("vital.kit:ready");
                    Vital::Tool::Event::emit("vital.core:ready");
                    set_process(true);
                });
                call_deferred("flush_deferred_queue");
            }
        });
    }

    void Core::_exit_tree() {
        kit_abort.store(true);
        kit_ready.store(false);
        if (kit_thread.joinable()) kit_thread.join();
        {
            std::lock_guard<std::mutex> lock(deferred_mutex);
            deferred_queue.clear();
        }
        if (!is_ready()) return;
        teardown();
        Vital::Tool::Event::emit("vital.core:free");
    }

    void Core::_process(double delta) {
        if (!is_ready()) return;
        {
            std::lock_guard<std::mutex> lock(deferred_mutex);
            if (!deferred_queue.empty()) call_deferred("flush_deferred_queue");
        }
        Manager::Sandbox::get_singleton() -> process(delta);
    }

    #if defined(Vital_SDK_Client)
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
        get_scene_tree() -> quit(0);
    }


    // Managers //
    bool Core::is_ready() {
        return Vital::is_runtime() && kit_ready.load();
    }

    void Core::teardown() {
        Manager::Asset::free_singleton();
        Model::teardown_spawner();
        #if defined(Vital_SDK_Client)
        free_environment();
        #endif
        Vital::Tool::Event::emit("vital.core:teardown");
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
            environment -> set_environment(env);
        }
        return environment -> get_environment();
    }

    void Core::free_environment() {
        if (!environment) return;
        environment -> queue_free();
        environment = nullptr;
    }
    #endif
}