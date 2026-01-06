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


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot {
    // Instantiators //
    Core::Core() {
        Sandbox::Lua::Singleton::fetch();
    }
    
    void Core::_ready() {
        singleton = singleton ? singleton : this;
        get_environment();
        Canvas::get_singleton();
        Sandbox::Lua::Singleton::fetch() -> ready();
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
    
    godot::RenderingServer* Core::get_rendering_server() {
        return godot::RenderingServer::get_singleton();
    }

    godot::ResourceLoader* Core::get_resource_loader() {
        return godot::ResourceLoader::get_singleton();
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
}