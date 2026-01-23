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
#include <Vital.extension/Engine/public/canvas.h>
#include <Vital.extension/Sandbox/index.h>
#include <Vital.sandbox/Tool/event.h>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot {
    // Instantiators //    
    void Core::_ready() {
        singleton = singleton ? singleton : this;
        set_process(true);
        set_process_unhandled_key_input(true); //set_process_unhandled_key_input
        Vital::Tool::Event::emit("vital.core:ready");
    }

    void Core::_exit_tree() {
        Vital::Tool::Event::emit("vital.core:free");
    }

    void Core::_process(double delta) {
        Sandbox::get_singleton() -> process(delta);
    }

    void Core::_unhandled_input(godot::Ref<godot::InputEvent> event) {
        auto* key_event = godot::Object::cast_to<godot::InputEventKey>(event.ptr());
        if (!key_event || key_event -> is_echo()) return;
        if (key_event->is_pressed()) {
            godot::Key key_code = static_cast<godot::Key>(key_event->get_keycode());
            godot::String key_name = godot::OS::get_singleton()->get_keycode_string(key_code);
            godot::UtilityFunctions::print("Key pressed: " + key_name);
        }
    }


    // Getters //
    Core* Core::get_singleton() {
        return singleton;
    }

    godot::Node* Core::get_root() {
        auto* tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton() -> get_main_loop());
        return tree ? tree -> get_root() : nullptr;
    }
    
    #if defined(Vital_SDK_Client)
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
}