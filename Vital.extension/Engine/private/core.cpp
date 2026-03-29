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
            // Only bounce to main thread if the node is still alive
            if (!kit_abort.load()) {
                call_deferred("on_kit_ready");
            }
        });
        // Do NOT detach — we need to join in _exit_tree
    }
    
    void Core::on_kit_ready() {
        Vital::print("sbox", "Core: Vital.kit ready — firing vital.core:ready");
        Vital::Tool::Event::emit("vital.core:ready");
        set_process(true);
    }

    void Core::_exit_tree() {
        // Signal the thread to skip the deferred call, then join before
        // any teardown — prevents call_deferred firing on a freed node
        kit_abort.store(true);
        if (kit_thread.joinable()) {
            kit_thread.join();
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


    // Getters //
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
            get_singleton() -> call_deferred("add_child", environment);
            godot::Ref<godot::Environment> env;
            env.instantiate();
            environment->set_environment(env);
        }
        return environment->get_environment();
    }
    #endif


    // Freers //
    #if defined(Vital_SDK_Client)
    void Core::free_environment() {
        if (!environment) return;
        environment->queue_free();
        environment = nullptr;
    }
    #endif


    // APIs //
    void Core::setup_model_spawner() {
        Model::setup_spawner();
    }

    void Core::spawn_model(const godot::String& name, int authority_peer) {
        Model::spawn_synced(to_std_string(name), authority_peer);
    }

    void Core::broadcast_asset_manifest(int peer_id) {
        #if !defined(Vital_SDK_Client)
        AssetManager::get_singleton() -> broadcast_manifest(peer_id);
        #endif
    }

    void Core::notify_resource_started(const godot::String& name) {
        #if !defined(Vital_SDK_Client)
        ResourceManager::get_singleton() -> notify_resource_started(Vital::to_std_string(name));
        #endif
    }

    void Core::notify_resource_stopped(const godot::String& name) {
        #if !defined(Vital_SDK_Client)
        ResourceManager::get_singleton() -> notify_resource_stopped(Vital::to_std_string(name));
        #endif
    }

    void Core::on_asset_downloaded(const godot::String& path) {
        #if defined(Vital_SDK_Client)
        const std::string p = Vital::to_std_string(path);
        Vital::Tool::Stack ready_args;
        ready_args.object["path"]   = Vital::Tool::StackValue(p);
        ready_args.object["cached"] = Vital::Tool::StackValue(false);
        Vital::Tool::Event::emit("asset:file_ready", ready_args);

        if (!AssetManager::get_singleton() -> is_downloading()) {
            Vital::print("sbox", "AssetManager: all assets ready");
            Vital::Tool::Event::emit("asset:ready", {});
        }
        #endif
    }

    void Core::on_asset_download_failed(const godot::String& path) {
        #if defined(Vital_SDK_Client)
        Vital::print("sbox", "AssetManager: download failed -> ", path.utf8().get_data());
        #endif
    }
}