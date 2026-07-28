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
    }

    int Audio_Bus::resolve_bus() const {
        if (bus_index < 0) return -1;
        return Engine::Core::get_audio_server() -> get_bus_index(godot::StringName(Tool::to_godot_string(bus_name)));
    }

    bool Audio_Bus::resolve_effect(int id, int& out_idx) const {
        out_idx = resolve_bus();
        if (out_idx < 0 || id < 0 || id >= Engine::Core::get_audio_server() -> get_bus_effect_count(out_idx)) return false;
        return true;
    }

    
    // Checkerss //
    bool Audio_Bus::is_effect_enabled(int id) const {
        int idx; 
        if (!resolve_effect(id, idx)) return false;
        return Engine::Core::get_audio_server() -> is_bus_effect_enabled(idx, id);
    }
    
    
    // Getters //
    godot::Ref<godot::AudioEffect> Audio_Bus::get_effect(int id) const {
        int idx; 
        if (!resolve_effect(id, idx)) return nullptr;
        return Engine::Core::get_audio_server() -> get_bus_effect(idx, id);
    }
        
    int Audio_Bus::get_effect_count() const {
        auto idx = resolve_bus();
        if (idx < 0) return 0;
        return Engine::Core::get_audio_server() -> get_bus_effect_count(idx);
    }


    // Setters //
    bool Audio_Bus::set_effect_enabled(int id, bool enabled) {
        int idx; 
        if (!resolve_effect(id, idx)) return false;
        Engine::Core::get_audio_server() -> set_bus_effect_enabled(idx, id, enabled);
        return true;
    }
    
    bool Audio_Bus::add_effect(const godot::Ref<godot::AudioEffect>& effect) {
        auto idx = resolve_bus();
        if (idx < 0 || !effect.is_valid()) return false;
        Engine::Core::get_audio_server() -> add_bus_effect(idx, effect);
        return true;
    }
    
    bool Audio_Bus::remove_effect(int id) {
        int idx; 
        if (!resolve_effect(id, idx)) return false;
        Engine::Core::get_audio_server() -> remove_bus_effect(idx, id);
        return true;
    }
}
#endif