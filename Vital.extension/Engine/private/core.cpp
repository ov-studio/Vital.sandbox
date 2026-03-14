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


////////////////////////////
// Vital: Engine: Engine //
////////////////////////////

namespace Vital::Engine {
    // Instantiators //    
    void Core::_ready() {
        singleton = singleton ? singleton : this;
        set_process(true);
        set_process_unhandled_key_input(get_platform() == "client");
        get_environment();
        if (Vital::is_editor()) return;
        Vital::Tool::Event::emit("vital.core:ready");
    }

    void Core::_exit_tree() {
        free_environment();
        if (Vital::is_editor()) return;
        Vital::Tool::Event::emit("vital.core:free");
    }

    void Core::_process(double delta) {
        if (Vital::is_editor()) return;
        Sandbox::get_singleton() -> process(delta);
    }

    void Core::_unhandled_input(godot::Ref<godot::InputEvent> event) {
        if (Vital::is_editor()) return;
        Sandbox::get_singleton() -> input(event);
    }


    // Getters //
    Core* Core::get_singleton() {
        return singleton;
    }

    godot::Node* Core::get_root() {
        auto tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton() -> get_main_loop());
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

    godot::Vector3 Core::get_screen_position_from_world(godot::Vector3 position, float padding) {
        godot::Vector3 result = {-1, -1, -1};
        auto viewport = get_singleton() -> get_viewport();
        auto camera = viewport ? viewport -> get_camera_3d() : nullptr;
        if (camera) {
            auto camera_position = camera -> get_global_position();
            auto camera_forward = -camera -> get_global_transform().basis.get_column(2);
            auto screen_position = camera -> unproject_position(position);
            auto screen_size = viewport -> get_visible_rect().size;
            if (
                (camera_forward.dot((position - camera_position).normalized()) > 0.0f) && 
                (screen_position.x >= -padding) && 
                (screen_position.y >= -padding) && 
                (screen_position.x <= screen_size.x + padding) && 
                (screen_position.y <= screen_size.y + padding)
            ) result = {screen_position.x, screen_position.y, camera_position.distance_to(position)};
        }
        return result;
    }
    #endif
}