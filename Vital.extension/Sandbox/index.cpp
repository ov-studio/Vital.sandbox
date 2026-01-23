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
        godot::UtilityFunctions::print("Initialized sandbox");
        vm = new Vital::Sandbox::Machine({
            {Vital::Sandbox::API::SSR::bind, Vital::Sandbox::API::SSR::inject},
            {Vital::Sandbox::API::SSAO::bind, Vital::Sandbox::API::SSAO::inject},
            {Vital::Sandbox::API::SSIL::bind, Vital::Sandbox::API::SSIL::inject},
            {Vital::Sandbox::API::SDFGI::bind, Vital::Sandbox::API::SDFGI::inject},
            {Vital::Sandbox::API::Emissive::bind, Vital::Sandbox::API::Emissive::inject},
            {Vital::Sandbox::API::Fog::bind, Vital::Sandbox::API::Fog::inject},
            {Vital::Sandbox::API::Volumetric_Fog::bind, Vital::Sandbox::API::Volumetric_Fog::inject},
            {Vital::Sandbox::API::Adjustment::bind, Vital::Sandbox::API::Adjustment::inject}
        });
    }

    Sandbox::~Sandbox() {
        godot::UtilityFunctions::print("uninitialized sandbox");
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
        auto* event_key = godot::Object::cast_to<godot::InputEventKey>(event.ptr());
        if (event_key) {
            if (event_key -> is_echo()) return;
            Vital::Tool::Stack arguments;
            arguments.object["keycode"] = to_std_string(godot::String::num_int64(event_key -> get_keycode()));
            arguments.object["state"] = event_key -> is_pressed();
            Vital::Tool::Event::emit("vital.sandbox:input", arguments);
        }
    }
    #endif
}