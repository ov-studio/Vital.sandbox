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
#include <Vital.extension/Engine/public/canvas.h>
#include <Vital.extension/Sandbox/api/canvas.h>
#include <Vital.extension/Sandbox/api/model.h>
#include <Vital.extension/Sandbox/api/webview.h>
#include <Vital.extension/Sandbox/api/font.h>
#include <Vital.extension/Sandbox/api/texture.h>
#include <Vital.extension/Sandbox/api/rendertarget.h>
#include <Vital.extension/Sandbox/api/gfx/ssr.h>
#include <Vital.extension/Sandbox/api/gfx/ssao.h>
#include <Vital.extension/Sandbox/api/gfx/ssil.h>
#include <Vital.extension/Sandbox/api/gfx/sdfgi.h>
#include <Vital.extension/Sandbox/api/gfx/emissive.h>
#include <Vital.extension/Sandbox/api/gfx/fog.h>
#include <Vital.extension/Sandbox/api/gfx/volumetric_fog.h>
#include <Vital.extension/Sandbox/api/gfx/adjustment.h>


/////////////////////////////
// Vital: Engine: Sandbox //
/////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Sandbox::Sandbox() {
        vm = new Vital::Sandbox::Machine({
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::Canvas>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::Model>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::Webview>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::Font>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::Texture>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::SVG>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::Rendertarget>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::SSAO>(),
            Vital::Sandbox::vm_module::make_api<Vital::Sandbox::API::SSR>(),
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


    // Managers //
    void Sandbox::ready() {
        signal("vital.sandbox:ready");
    }

    void Sandbox::process(double delta) {
        signal("vital.sandbox:process", Vital::Tool::StackValue(delta));
    }

    #if defined(Vital_SDK_Client)
    void Sandbox::draw(Canvas* canvas) {
        signal("vital.sandbox:draw");
    }

    void Sandbox::input(godot::Ref<godot::InputEvent> event) {
        if (auto event_key = godot::Object::cast_to<godot::InputEventKey>(event.ptr())) {
            if (event_key -> is_echo()) return;
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