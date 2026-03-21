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
#include <Vital.extension/Engine/public/model.h>
#include <Vital.extension/Engine/public/canvas.h>
#include <Vital.extension/Sandbox/index.h>


////////////////////////////
// Vital: Engine: Engine //
////////////////////////////

namespace Vital::Engine {
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


    // Teardown //
    void Core::teardown() {
        Model::teardown_spawner();
    }
}