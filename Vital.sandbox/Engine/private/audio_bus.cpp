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
        bus_name = prefix + std::to_string(reinterpret_cast<uintptr_t>(this));
        Engine::Core::get_audio_server() -> add_bus();
        bus_index = Engine::Core::get_audio_server() -> get_bus_count() - 1;
        Engine::Core::get_audio_server() -> set_bus_name(bus_index, Tool::to_godot_string(bus_name));
        Engine::Core::get_audio_server() -> set_bus_send(bus_index, godot::StringName("Master"));
        return bus_name;
    }

    void Audio_Bus::destroy_bus() {
        if (bus_index < 0) return;
        auto idx = Engine::Core::get_audio_server() -> get_bus_index(godot::StringName(Tool::to_godot_string(bus_name)));
        if (idx >= 0) Engine::Core::get_audio_server() -> remove_bus(idx);
        bus_index = -1;
        effect_names.clear();
    }

    int Audio_Bus::resolve_bus() const {
        if (bus_index < 0) return -1;
        return Engine::Core::get_audio_server() -> get_bus_index(godot::StringName(Tool::to_godot_string(bus_name)));
    }

    bool Audio_Bus::resolve_effect(const std::string& name, int& out_bus_idx, int& out_slot_idx) const {
        out_bus_idx = resolve_bus();
        if (out_bus_idx < 0) return false;
        auto it = std::find(effect_names.begin(), effect_names.end(), name);
        if (it == effect_names.end()) return false;
        out_slot_idx = static_cast<int>(std::distance(effect_names.begin(), it));
        if (out_slot_idx >= Engine::Core::get_audio_server() -> get_bus_effect_count(out_bus_idx)) return false;
        return true;
    }


    // Checkers //
    bool Audio_Bus::is_effect_enabled(const std::string& name) const {
        int bus_idx, slot_idx;
        if (!resolve_effect(name, bus_idx, slot_idx)) return false;
        return Engine::Core::get_audio_server() -> is_bus_effect_enabled(bus_idx, slot_idx);
    }


    // Getters //
    godot::Ref<godot::AudioEffect> Audio_Bus::get_effect(const std::string& name) const {
        int bus_idx, slot_idx;
        if (!resolve_effect(name, bus_idx, slot_idx)) return nullptr;
        return Engine::Core::get_audio_server() -> get_bus_effect(bus_idx, slot_idx);
    }

    int Audio_Bus::get_effect_count() const {
        auto idx = resolve_bus();
        if (idx < 0) return 0;
        return Engine::Core::get_audio_server() -> get_bus_effect_count(idx);
    }

    std::vector<std::string> Audio_Bus::get_effects() const {
        return effect_names;
    }


    // Setters //
    bool Audio_Bus::set_effect_enabled(const std::string& name, bool enabled) {
        int bus_idx, slot_idx;
        if (!resolve_effect(name, bus_idx, slot_idx)) return false;
        Engine::Core::get_audio_server() -> set_bus_effect_enabled(bus_idx, slot_idx, enabled);
        return true;
    }


    // Misc //
    bool Audio_Bus::add_effect(const std::string& name, const godot::Ref<godot::AudioEffect>& effect) {
        auto idx = resolve_bus();
        if (idx < 0 || !effect.is_valid()) return false;
        auto it = std::find(effect_names.begin(), effect_names.end(), name);
        if (it != effect_names.end()) return false;
        Engine::Core::get_audio_server() -> add_bus_effect(idx, effect);
        effect_names.push_back(name);
        return true;
    }

    bool Audio_Bus::remove_effect(const std::string& name) {
        int bus_idx, slot_idx;
        if (!resolve_effect(name, bus_idx, slot_idx)) return false;
        Engine::Core::get_audio_server() -> remove_bus_effect(bus_idx, slot_idx);
        effect_names.erase(effect_names.begin() + slot_idx);
        return true;
    }
}
#endif