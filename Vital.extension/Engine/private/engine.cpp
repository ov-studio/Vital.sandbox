/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: engine.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Engine Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/engine.h>
#include <Vital.extension/Engine/public/canvas.h>
#include <Vital.extension/Sandbox/lua/public/index.h>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot {
    // Instantiators //
    Engine2::Engine2() {
        Vital::Godot::Sandbox::Lua::Singleton::fetch();
    }
    
    void Engine2::_ready() {
        get_environment();
        Vital::Godot::Sandbox::Lua::Singleton::fetch() -> ready();
        //if (!godot::Engine::get_singleton() -> is_editor_hint()) {
            auto* root = get_tree() -> get_root();
            canvas = memnew(Vital::Godot::Canvas);
            root -> call_deferred("add_child", canvas);
        //}
    }

    void Engine2::_process(double delta) {
        Vital::Godot::Sandbox::Lua::Singleton::fetch() -> process(delta);
    }


    // Getters //
    godot::Node* Engine2::get_root() {
        auto* tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton() -> get_main_loop());
        return tree ? tree -> get_root() : nullptr;
    }
    
    godot::ResourceLoader* Engine2::get_resource_loader() {
        return godot::ResourceLoader::get_singleton();
    }
    
    godot::Ref<godot::Environment> Engine2::get_environment() {
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