/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_bus.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Audio Bus Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/audio_bus.h>


///////////////////////////////
// Vital: Engine: Audio_Bus //
///////////////////////////////

namespace Vital::Engine {
    // Helpers //
    const std::string& Audio_Bus::create_bus(const std::string& prefix) {
        auto server = godot::AudioServer::get_singleton();
        bus_name = prefix + std::to_string(reinterpret_cast<uintptr_t>(this));
        server -> add_bus();
        bus_index = server -> get_bus_count() - 1;
        server -> set_bus_name(bus_index, Tool::to_godot_string(bus_name));
        server -> set_bus_send(bus_index, godot::StringName("Master"));
        return bus_name;
    }

    void Audio_Bus::destroy_bus() {
        if (bus_index < 0) return;
        auto server = godot::AudioServer::get_singleton();
        auto current_index = server -> get_bus_index(godot::StringName(Tool::to_godot_string(bus_name)));
        if (current_index >= 0) server -> remove_bus(current_index);
        bus_index = -1;
    }

    int32_t Audio_Bus::resolve_bus() const {
        if (bus_index < 0) return -1;
        return godot::AudioServer::get_singleton() -> get_bus_index(godot::StringName(Tool::to_godot_string(bus_name)));
    }

    
    // Checkerss //
    bool Audio_Bus::is_effect_enabled(int32_t effect) const {
        auto idx = resolve_bus();
        auto server = godot::AudioServer::get_singleton();
        if (idx < 0 || effect < 0 || effect >= server -> get_bus_effect_count(idx)) return false;
        return server -> is_bus_effect_enabled(idx, effect);
    }


    // Getters //
    godot::Ref<godot::AudioEffect> Audio_Bus::get_effect(int32_t effect) const {
        auto idx = resolve_bus();
        auto server = godot::AudioServer::get_singleton();
        if (idx < 0 || effect < 0 || effect >= server -> get_bus_effect_count(idx)) return nullptr;
        return server -> get_bus_effect(idx, effect);
    }

    int32_t Audio_Bus::get_effect_count() const {
        auto idx = resolve_bus();
        if (idx < 0) return 0;
        return godot::AudioServer::get_singleton() -> get_bus_effect_count(idx);
    }


    // Setters //
    bool Audio_Bus::set_effect_enabled(int32_t effect, bool enabled) {
        auto idx = resolve_bus();
        auto server = godot::AudioServer::get_singleton();
        if (idx < 0 || effect < 0 || effect >= server -> get_bus_effect_count(idx)) return false;
        server -> set_bus_effect_enabled(idx, effect, enabled);
        return true;
    }


    // Misc //
    bool Audio_Bus::add_effect(const godot::Ref<godot::AudioEffect>& effect) {
        auto idx = resolve_bus();
        if (idx < 0 || !effect.is_valid()) return false;
        godot::AudioServer::get_singleton() -> add_bus_effect(idx, effect);
        return true;
    }

    bool Audio_Bus::remove_effect(int32_t effect) {
        auto idx = resolve_bus();
        auto server = godot::AudioServer::get_singleton();
        if (idx < 0 || effect < 0 || effect >= server -> get_bus_effect_count(idx)) return false;
        server -> remove_bus_effect(idx, effect);
        return true;
    }
}
#endif