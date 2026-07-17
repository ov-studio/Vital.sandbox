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
#include <Vital.sandbox/Engine/public/console.h>


/////////////////////////////
// Vital: Manager: Sandbox //
/////////////////////////////

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
    void Sandbox::ready() {
        signal("sandbox:ready");
    }

    void Sandbox::drain() {
        Vital::Sandbox::Machine::drain();
    }

    void Sandbox::process(double delta) {
        signal("sandbox:process", Tool::StackValue(delta));
    }

    #if defined(VSDK_Client)
    void Sandbox::draw(Engine::Canvas* canvas) {
        signal("sandbox:draw");
    }

    void Sandbox::input(godot::Ref<godot::InputEvent> event) {
        if (auto event_key = godot::Object::cast_to<godot::InputEventKey>(event.ptr())) {
            if (event_key -> is_echo()) return;
            if (event_key -> is_pressed() && Engine::Console::get_singleton() -> on_key(event_key -> get_keycode())) return;
            signal("sandbox:key_input",
                Tool::StackValue(static_cast<int32_t>(event_key -> get_keycode())),
                Tool::StackValue(event_key -> is_pressed())
            );
        }
        else if (auto event_mouse_button = godot::Object::cast_to<godot::InputEventMouseButton>(event.ptr())) {
            signal("sandbox:key_input",
                Tool::StackValue(static_cast<int32_t>(event_mouse_button -> get_button_index())),
                Tool::StackValue(event_mouse_button -> is_pressed())
            );
        }
        else if (auto event_mouse_motion = godot::Object::cast_to<godot::InputEventMouseMotion>(event.ptr())) {
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
