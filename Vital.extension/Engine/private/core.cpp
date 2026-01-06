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
        Vital::Godot::Sandbox::Lua::Singleton::fetch();
    }
    
    void Core::_ready() {
        get_environment();
        Vital::Godot::Sandbox::Lua::Singleton::fetch() -> ready();
        //if (!godot::Engine::get_singleton() -> is_editor_hint()) {
            auto* root = get_tree() -> get_root();
            canvas = memnew(Vital::Godot::Canvas);
            root -> call_deferred("add_child", canvas);
        //}
    }

    void Core::_process(double delta) {
        Vital::Godot::Sandbox::Lua::Singleton::fetch() -> process(delta);
    }


    // Getters //
    godot::Node* Core::get_root() {
        auto* tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton() -> get_main_loop());
        return tree ? tree -> get_root() : nullptr;
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