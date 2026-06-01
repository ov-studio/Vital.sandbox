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


    // Managers //
    void Sandbox::ready() {
        signal("vital.sandbox:ready");
    }

    void Sandbox::drain() {
        Vital::Sandbox::Machine::drain();
    }

    void Sandbox::process(double delta) {
        signal("vital.sandbox:process", Tool::StackValue(delta));
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
                Tool::StackValue(Tool::to_std_string(godot::String::num_int64(event_key -> get_keycode()))),
                Tool::StackValue(event_key -> is_pressed())
            );
        }
        else if (auto event_mouse_button = godot::Object::cast_to<godot::InputEventMouseButton>(event.ptr())) {
            signal("vital.sandbox:key_input",
                Tool::StackValue(fmt::format("mouse_{}", Tool::to_std_string(godot::String::num_int64(event_mouse_button -> get_button_index())))),
                Tool::StackValue(event_mouse_button -> is_pressed())
            );
        }
        else if (auto event_mouse_motion = godot::Object::cast_to<godot::InputEventMouseMotion>(event.ptr())) {
            auto position = event_mouse_motion -> get_position();
            signal("vital.sandbox:mouse_move",
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


    // Exports //
    int Sandbox::get_export_ref(const std::string& resource, const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex);
        auto rit = exports.find(resource);
        if (rit == exports.end()) return LUA_NOREF;
        auto fit = rit -> second.find(name);
        if (fit == rit -> second.end()) return LUA_NOREF;
        return fit -> second;
    }
    
    bool Sandbox::register_export(const std::string& resource, const std::string& name, int reference) {
        std::lock_guard<std::mutex> lock(mutex);
        auto& map = exports[resource];
        auto it = map.find(name);
        if (it != map.end()) {
            vm -> del_raw_reference(it -> second);
            map.erase(it);
        }
        map[name] = reference;
        return true;
    }

    std::vector<std::string> Sandbox::list_exports(const std::string& resource) const {
        std::lock_guard<std::mutex> lock(mutex);
        std::vector<std::string> result;
        auto it = exports.find(resource);
        if (it == exports.end()) return result;
        result.reserve(it -> second.size());
        for (const auto& [name, _] : it -> second) result.push_back(name);
        return result;
    }

    void Sandbox::reset_exports(const std::string& resource) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = exports.find(resource);
        if (it == exports.end()) return;
        for (auto& [name, ref] : it -> second) vm -> del_raw_reference(ref);
        exports.erase(it);
    }
}