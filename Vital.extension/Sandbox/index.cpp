/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: index.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/index.h>
#include <Vital.extension/Sandbox/api/ssr.h>
#include <Vital.extension/Sandbox/api/ssao.h>
#include <Vital.extension/Sandbox/api/ssil.h>
#include <Vital.extension/Sandbox/api/sdfgi.h>
#include <Vital.extension/Sandbox/api/emissive.h>
#include <Vital.extension/Sandbox/api/fog.h>
#include <Vital.extension/Sandbox/api/volumetric_fog.h>
#include <Vital.extension/Sandbox/api/adjustment.h>


////////////////////////////
// Vital: Godot: Sandbox //
////////////////////////////

namespace Vital::Godot {
    // Instantiators //
    Sandbox::Sandbox() {
        vm = new Vital::Sandbox::Machine({
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::SSR>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::SSAO>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::SSIL>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::SDFGI>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::Emissive>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::Fog>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::Volumetric_Fog>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::Adjustment>()
        });
    }

    Sandbox::~Sandbox() {
        if (!vm) return;
        delete vm;
        vm = nullptr;
    }


    // Getters //
    Sandbox* Sandbox::get_singleton() {
        singleton = singleton ? singleton : memnew(Sandbox());
        return singleton;
    }

    void Sandbox::free_singleton() {
        if (!singleton) return
        singleton -> queue_free();
        singleton = nullptr;
    }


    // APIs //
    void Sandbox::ready() {
        Vital::Tool::Event::emit("vital.sandbox:ready");
    }

    void Sandbox::process(double delta) {
        Vital::Tool::Stack arguments;
        arguments.object["delta"] = Vital::Tool::StackValue(delta);
        Vital::Tool::Event::emit("vital.sandbox:process", arguments);
    }

    #if defined(Vital_SDK_Client)
    void Sandbox::draw(Canvas* canvas) {
        Vital::Tool::Event::emit("vital.sandbox:draw");
    }

    void Sandbox::input(godot::Ref<godot::InputEvent> event) {
        if (auto* event_key = godot::Object::cast_to<godot::InputEventKey>(event.ptr())) {
            if (event_key -> is_echo()) return;
            Vital::Tool::Stack arguments;
            arguments.object["keycode"] = to_std_string(godot::String::num_int64(event_key -> get_keycode()));
            arguments.object["state"] = event_key -> is_pressed();
            Vital::Tool::Event::emit("vital.sandbox:key_input", arguments);
        }
        else if (auto* event_mouse_button = godot::Object::cast_to<godot::InputEventMouseButton>(event.ptr())) {
            Vital::Tool::Stack arguments;
            arguments.object["keycode"] = fmt::format("mouse_{}", to_std_string(godot::String::num_int64(event_mouse_button -> get_button_index())));
            arguments.object["state"] = event_mouse_button -> is_pressed();
            Vital::Tool::Event::emit("vital.sandbox:key_input", arguments);
        }
        else if (auto* event_mouse_motion = godot::Object::cast_to<godot::InputEventMouseMotion>(event.ptr())) {
            auto position = event_mouse_motion -> get_position();
            Vital::Tool::Stack arguments;
            arguments.object["x"] = position.x;
            arguments.object["y"] = position.y;
            Vital::Tool::Event::emit("vital.sandbox:mouse_move", arguments);
        }
    }
    #endif
}