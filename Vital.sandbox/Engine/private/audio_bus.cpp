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
        fx_buffer.clear();
    }

    int Audio_Bus::resolve_bus() const {
        if (bus_index < 0) return -1;
        return Engine::Core::get_audio_server() -> get_bus_index(godot::StringName(Tool::to_godot_string(bus_name)));
    }

    bool Audio_Bus::resolve_fx(const std::string& name, int& out_bus_idx, int& out_slot_idx) const {
        out_bus_idx = resolve_bus();
        if (out_bus_idx < 0) return false;
        auto it = std::find(fx_buffer.begin(), fx_buffer.end(), name);
        if (it == fx_buffer.end()) return false;
        out_slot_idx = static_cast<int>(std::distance(fx_buffer.begin(), it));
        if (out_slot_idx >= Engine::Core::get_audio_server() -> get_bus_effect_count(out_bus_idx)) return false;
        return true;
    }

    godot::Ref<godot::AudioStream> Audio_Bus::load_stream_from_buffer(const godot::PackedByteArray& buffer) {
        godot::Ref<godot::AudioStream> loaded;
        switch (Tool::Format::get_format(format_registry, Format::UNKNOWN, buffer)) {
            case Format::OGG:     loaded = godot::AudioStreamOggVorbis::load_from_buffer(buffer); break;
            case Format::WAV:     loaded = godot::AudioStreamWAV::load_from_buffer(buffer);       break;
            case Format::MP3:
            case Format::UNKNOWN:
            default:              loaded = godot::AudioStreamMP3::load_from_buffer(buffer);       break;
        }
        if (!loaded.is_valid()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid audio buffer");
        return loaded;
    }

    bool Audio_Bus::get_stream_loop(const godot::Ref<godot::AudioStream>& stream) {
        if (!stream.is_valid()) return false;
        if (auto ogg = godot::Object::cast_to<godot::AudioStreamOggVorbis>(stream.ptr())) return ogg -> has_loop();
        if (auto wav = godot::Object::cast_to<godot::AudioStreamWAV>(stream.ptr())) return wav -> get_loop_mode() != godot::AudioStreamWAV::LOOP_DISABLED;
        if (auto mp3 = godot::Object::cast_to<godot::AudioStreamMP3>(stream.ptr())) return mp3 -> has_loop();
        return false;
    }

    void Audio_Bus::set_stream_loop(const godot::Ref<godot::AudioStream>& stream, bool loop) {
        if (!stream.is_valid()) return;
        if (auto ogg = godot::Object::cast_to<godot::AudioStreamOggVorbis>(stream.ptr())) ogg -> set_loop(loop);
        else if (auto wav = godot::Object::cast_to<godot::AudioStreamWAV>(stream.ptr())) wav -> set_loop_mode(loop ? godot::AudioStreamWAV::LOOP_FORWARD : godot::AudioStreamWAV::LOOP_DISABLED);
        else if (auto mp3 = godot::Object::cast_to<godot::AudioStreamMP3>(stream.ptr())) mp3 -> set_loop(loop);
    }


    // Checkers //
    bool Audio_Bus::is_looped() const {
        return get_stream_loop(get_stream());
    }

    bool Audio_Bus::is_fx_enabled(const std::string& name) const {
        int bus_idx, slot_idx;
        if (!resolve_fx(name, bus_idx, slot_idx)) return false;
        return Engine::Core::get_audio_server() -> is_bus_effect_enabled(bus_idx, slot_idx);
    }


    // Getters //
    godot::Ref<godot::AudioEffect> Audio_Bus::get_fx(const std::string& name) const {
        int bus_idx, slot_idx;
        if (!resolve_fx(name, bus_idx, slot_idx)) return nullptr;
        return Engine::Core::get_audio_server() -> get_bus_effect(bus_idx, slot_idx);
    }

    std::vector<std::string> Audio_Bus::list_effects() const {
        return fx_buffer;
    }


    // Setters //
    bool Audio_Bus::set_looped(bool loop) {
        auto stream = get_stream();
        if (!stream.is_valid()) return false;
        set_stream_loop(stream, loop);
        return true;
    }

    bool Audio_Bus::set_fx_enabled(const std::string& name, bool state) {
        int bus_idx, slot_idx;
        if (!resolve_fx(name, bus_idx, slot_idx)) return false;
        Engine::Core::get_audio_server() -> set_bus_effect_enabled(bus_idx, slot_idx, state);
        return true;
    }


    // Misc //
    bool Audio_Bus::add_fx(const std::string& name, const godot::Ref<godot::AudioEffect>& effect) {
        auto idx = resolve_bus();
        if (idx < 0 || !effect.is_valid()) return false;
        auto it = std::find(fx_buffer.begin(), fx_buffer.end(), name);
        if (it != fx_buffer.end()) return false;
        Engine::Core::get_audio_server() -> add_bus_effect(idx, effect);
        fx_buffer.push_back(name);
        return true;
    }

    bool Audio_Bus::remove_fx(const std::string& name) {
        int bus_idx, slot_idx;
        if (!resolve_fx(name, bus_idx, slot_idx)) return false;
        Engine::Core::get_audio_server() -> remove_bus_effect(bus_idx, slot_idx);
        fx_buffer.erase(fx_buffer.begin() + slot_idx);
        return true;
    }
}
#endif
