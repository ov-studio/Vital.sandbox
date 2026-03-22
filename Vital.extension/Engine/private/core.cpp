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
#include <Vital.extension/Engine/public/network.h>
#include <Vital.extension/Engine/public/asset.h>
#include <Vital.extension/Engine/public/model.h>
#include <Vital.extension/Engine/public/canvas.h>
#include <Vital.extension/Sandbox/index.h>


////////////////////////////
// Vital: Engine: Engine //
////////////////////////////

namespace Vital::Engine {
    // TODO: Improve
    // Instantiators //
    void Core::_ready() {
        singleton = singleton ? singleton : this;
        set_process(true);
        #if defined(Vital_SDK_Client)
        set_process_unhandled_key_input(true);
        get_environment();
        #endif
        if (!Vital::is_runtime()) return;
        Vital::Tool::Event::emit("vital.core:ready");
    }

    void Core::_exit_tree() {
        #if defined(Vital_SDK_Client)
        free_environment();
        #endif
        if (!Vital::is_runtime()) return;
        teardown();
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


    // Getters //
    Core* Core::get_singleton() {
        return singleton;
    }

    godot::Node* Core::get_root() {
        auto tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton() -> get_main_loop());
        return tree ? tree -> get_root() : nullptr;
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


    // Helpers //
    void Core::add_child_node(godot::Node* node) {
        add_child(node);
    }

    void Core::setup_model_spawner() {
        Model::setup_spawner();
    }

    void Core::spawn_model(const godot::String& name, int authority_peer) {
        Model::spawn_synced(to_std_string(name), authority_peer);
    }

    void Core::process_asset_chunk(const godot::String& path) {
        AssetManager::get_singleton()->process_chunk(to_std_string(path));
    }

    void Core::send_asset_to_peer(const godot::String& path, int peer_id) {
        AssetManager::get_singleton()->send_asset(to_std_string(path), peer_id);
    }

    void Core::send_asset_chunk(const godot::String& path, const godot::String& hash, const godot::String& data, int chunk_index, int chunk_total, int peer_id) {
        Vital::Tool::Stack msg;
        msg.object["type"]        = Vital::Tool::StackValue(std::string("asset:chunk"));
        msg.object["path"]        = Vital::Tool::StackValue(to_std_string(path));
        msg.object["hash"]        = Vital::Tool::StackValue(to_std_string(hash));
        msg.object["data"]        = Vital::Tool::StackValue(to_std_string(data));
        msg.object["chunk_index"] = Vital::Tool::StackValue((int32_t)chunk_index);
        msg.object["chunk_total"] = Vital::Tool::StackValue((int32_t)chunk_total);
        Vital::Engine::Network::get_singleton()->send(msg, peer_id);
    }

    void Core::on_asset_saved(const godot::String& path) {
        AssetManager::get_singleton()->on_asset_saved(to_std_string(path));
    }


    // Teardown //
    void Core::teardown() {
        AssetManager::free_singleton();
        Model::teardown_spawner();
    }
}