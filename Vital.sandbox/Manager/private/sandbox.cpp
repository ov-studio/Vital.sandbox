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
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/canvas.h>
#include <Vital.sandbox/Engine/public/console.h>


/////////////////////////////
// Vital: Manager: Sandbox //
/////////////////////////////

// TODO: Scope under Manager namespace
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


    // Utils //
    Sandbox* Sandbox::get_singleton() {
        singleton = singleton ? singleton : memnew(Sandbox());
        return singleton;
    }

    void Sandbox::free_singleton() {
        if (!singleton) return;
        singleton -> queue_free();
        singleton = nullptr;
    }


    // Getters //
    Vital::Sandbox::Machine* Sandbox::get_vm() {
        return vm;
    }


    // APIs //
    void Sandbox::ready() {
        signal("vital.sandbox:ready");
    }

    void Sandbox::process(double delta) {
        signal("vital.sandbox:process", Vital::Tool::StackValue(delta));
    }

    #if defined(Vital_SDK_Client)
    void Sandbox::draw(Engine::Canvas* canvas) {
        signal("vital.sandbox:draw");
    }

    void Sandbox::input(godot::Ref<godot::InputEvent> event) {
        if (auto event_key = godot::Object::cast_to<godot::InputEventKey>(event.ptr())) {
            if (event_key -> is_echo()) return;
            if (event_key -> is_pressed() && Engine::Console::get_singleton() -> on_key(event_key -> get_keycode())) return;
            signal("vital.sandbox:key_input",
                Vital::Tool::StackValue(to_std_string(godot::String::num_int64(event_key -> get_keycode()))),
                Vital::Tool::StackValue(event_key -> is_pressed())
            );
        }
        else if (auto event_mouse_button = godot::Object::cast_to<godot::InputEventMouseButton>(event.ptr())) {
            signal("vital.sandbox:key_input",
                Vital::Tool::StackValue(fmt::format("mouse_{}", to_std_string(godot::String::num_int64(event_mouse_button -> get_button_index())))),
                Vital::Tool::StackValue(event_mouse_button -> is_pressed())
            );
        }
        else if (auto event_mouse_motion = godot::Object::cast_to<godot::InputEventMouseMotion>(event.ptr())) {
            auto position = event_mouse_motion -> get_position();
            signal("vital.sandbox:mouse_move",
                Vital::Tool::StackValue(position.x),
                Vital::Tool::StackValue(position.y)
            );
        }
    }
    #endif
}