/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: sandbox.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/bind.h>
#include <Vital.sandbox/Engine/public/webview.h>

//////////////////////////////
// Vital: Manager: Sandbox //
//////////////////////////////

namespace Vital::Manager {
    // Instantiators //
    Sandbox::Sandbox() {
        vm = new Vital::Sandbox::Machine();
    }

    Sandbox::~Sandbox() {
        if (!vm) return;
        delete vm;
        vm = nullptr;
    }


    // Singleton //
    void Sandbox::free_singleton() {
        if (!singleton) return;
        singleton -> teardown();
        singleton -> queue_free();
        singleton = nullptr;
    }


    // Managers //
    void Sandbox::drain() {
        Vital::Sandbox::Machine::drain();
    }

    void Sandbox::process(double delta) {
        #if defined(VSDK_Client)
        Engine::Bind::get_singleton() -> poll(delta);
        if (!Engine::Core::get_singleton() -> is_sandbox_ui_ready()) return;
        #endif
        signal("sandbox:process", Tool::StackValue(delta));
    }

    void Sandbox::physics_process(double delta) {
        #if defined(VSDK_Client)
        if (!Engine::Core::get_singleton() -> is_sandbox_ui_ready()) return;
        #endif
        signal("sandbox:physics_process", Tool::StackValue(delta));
    }

    #if defined(VSDK_Client)
    void Sandbox::draw(Engine::Canvas* canvas) {
        if (!Engine::Core::get_singleton() -> is_sandbox_ui_ready()) return;
        signal("sandbox:draw");
    }

    void Sandbox::input(godot::Ref<godot::InputEvent> event) {
        const bool forward_input = !Engine::Webview::get_key_forwarder();
        if (auto event_key = godot::Object::cast_to<godot::InputEventKey>(event.ptr())) {
            if (event_key -> is_echo()) return;
            if (Engine::Bind::get_singleton() -> handle_key(event_key)) {
                Engine::Core::get_scene_root() -> set_input_as_handled();
                return;
            }

            if (!forward_input) return;
            signal("sandbox:key_input",
                Tool::StackValue(static_cast<int32_t>(event_key -> get_keycode())),
                Tool::StackValue(event_key -> is_pressed())
            );
        }
        else if (auto event_mouse_button = godot::Object::cast_to<godot::InputEventMouseButton>(event.ptr())) {
            if (!forward_input) return;
            signal("sandbox:key_input",
                Tool::StackValue(static_cast<int32_t>(event_mouse_button -> get_button_index())),
                Tool::StackValue(event_mouse_button -> is_pressed())
            );
        }
        else if (auto event_mouse_motion = godot::Object::cast_to<godot::InputEventMouseMotion>(event.ptr())) {
            if (!forward_input) return;
            auto position = event_mouse_motion -> get_position();
            signal("sandbox:mouse_move",
                Tool::StackValue(position.x),
                Tool::StackValue(position.y)
            );
        }
    }
    #endif


    // Getters //
    Vital::Sandbox::Machine* Sandbox::get_vm() {
        return vm;
    }
}
