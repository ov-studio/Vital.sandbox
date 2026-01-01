/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: index.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Engine Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/index.h>
#include <Vital.extension/Sandbox/lua/public/index.h>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot::Engine {
    // Instantiators //
    Singleton::Singleton() {
        Vital::Godot::Sandbox::Lua::Singleton::fetch();
    }
    
    void Singleton::_bind_methods() {

    }
    
    void Singleton::_ready() {
        Vital::Godot::Sandbox::Lua::Singleton::fetch() -> ready();
    }

    void Singleton::_process(double delta) {
        Vital::Godot::Sandbox::Lua::Singleton::fetch() -> process(delta);
    }


    // Getters //
    godot::Node* Singleton::get_root() {
        return godot::Engine::get_singleton() -> is_editor_hint() ? godot::EditorInterface::get_singleton() -> get_edited_scene_root() : godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton() -> get_main_loop()) -> get_root();
    }

    godot::ResourceLoader* Singleton::get_resource_loader() {
        return godot::ResourceLoader::get_singleton();
    }

    godot::Ref<godot::Environment> Singleton::get_environment() {
        godot::Node* root = get_root();
        godot::WorldEnvironment* parent = nullptr;
        std::vector<godot::WorldEnvironment*> nodes;
        fetch_nodes_by_type(root, nodes, 1);
        if (!nodes.empty()) parent = nodes[0];
        else {
            parent = memnew(godot::WorldEnvironment);
            root -> add_child(parent);
            godot::Ref<godot::Environment> env;
            env.instantiate();
            parent -> set_environment(env);
        }
        return parent -> get_environment();
    }
}