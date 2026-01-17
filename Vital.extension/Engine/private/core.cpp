/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: core.cpp
     Author: vStudio
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
#include <Vital.extension/Sandbox/lua/public/index.h>
#include <Vital.sandbox/Tool/event.h>
#include <Vital.sandbox/System/public/discord.h>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot {
    // Instantiators //
    Core::Core() {
        Sandbox::Lua::Singleton::fetch();

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
        Vital::Tool::Stack arguments;
        Vital::System::Event::emit("Godot:Core:@ready", arguments);
    }

    void Core::_process(double delta) {
        Sandbox::Lua::Singleton::fetch() -> process(delta);
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
        auto* root = get_root();
        godot::WorldEnvironment* parent = nullptr;
        std::vector<godot::WorldEnvironment*> nodes;
        fetch_nodes_by_type(root, nodes, 1);
        if (!nodes.empty()) parent = nodes[0];
        else {
            parent = memnew(godot::WorldEnvironment);
            root -> call_deferred("add_child", parent);
            godot::Ref<godot::Environment> env;
            env.instantiate();
            parent -> set_environment(env);
        }
        return parent -> get_environment();
    }
    #endif
}