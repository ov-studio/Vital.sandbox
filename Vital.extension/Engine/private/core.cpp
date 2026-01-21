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
#include <Vital.sandbox/System/public/discord.h>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot {
    // Instantiators //
    Core::Core() {
        #if defined(Vital_SDK_Client)
        // Initialize Discord Rich Presence
        if (Vital::System::Discord::start()) {
            godot::UtilityFunctions::print("Discord Rich Presence initialized");

            if(Vital::System::Discord::setActivity("In Main Menu", "Thinking about what to do..")){
                godot::UtilityFunctions::print("Discord Rich Presence activity set");
            } else {
                godot::UtilityFunctions::print("Failed to set Discord Rich Presence activity");
            }
        } else {
            godot::UtilityFunctions::print("Failed to initialize Discord (Maybe discord is not running?)");
        }
        #endif
    }
    
    void Core::_ready() {
        singleton = singleton ? singleton : this;
        if (Vital::is_editor()) return;
        Vital::Tool::Event::emit("vital.core:ready");
    }

    void Core::_process(double delta) {
        Sandbox::get_singleton() -> process(delta);
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