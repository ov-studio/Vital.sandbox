/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_2d.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     Desc: Audio 2D Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/audio_2d.h>


//////////////////////////////
// Vital: Engine: Audio_2D //
//////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Audio2D::Audio2D(const godot::Ref<godot::AudioStream>& stream, bool autoplay) {
        this -> stream = stream;
        player = memnew(godot::AudioStreamPlayer2D);
        player -> set_stream(stream);
        player -> set_autoplay(autoplay);
        Engine::Core::get_singleton() -> add_child(player);
        init_bus();
    }

    Audio2D::~Audio2D() {
        destroy_bus();
        if (!player) return;
        player -> queue_free();
        player = nullptr;
    }


    // Bus //
    void Audio2D::init_bus() {
        auto* server = godot::AudioServer::get_singleton();
        bus_name = "vs_audio2d_" + std::to_string(reinterpret_cast<uintptr_t>(this));
        server -> add_bus();
        bus_index = server -> get_bus_count() - 1;
        server -> set_bus_name(bus_index, godot::String(bus_name.c_str()));
        server -> set_bus_send(bus_index, godot::StringName("Master"));
        player -> set_bus(godot::StringName(bus_name.c_str()));
    }

    void Audio2D::destroy_bus() {
        auto* server = godot::AudioServer::get_singleton();
        if (bus_index < 0) return;
        auto current_index = server -> get_bus_index(godot::StringName(bus_name.c_str()));
        if (current_index >= 0) server -> remove_bus(current_index);
        bus_index = -1;
    }


    // Managers //
    Audio2D* Audio2D::create(const std::string& base, const std::string& path, bool autoplay) {
        return create_from_buffer(Tool::File::read_binary(base, path), autoplay);
    }

    Audio2D* Audio2D::create_from_buffer(const godot::PackedByteArray& buffer, bool autoplay) {
        godot::Ref<godot::AudioStream> loaded;
        switch (Tool::Format::get_format(format_registry, Format::UNKNOWN, buffer)) {
            case Format::OGG:     loaded = godot::AudioStreamOggVorbis::load_from_buffer(buffer); break;
            case Format::WAV:     loaded = godot::AudioStreamWAV::load_from_buffer(buffer);       break;
            case Format::MP3:
            case Format::UNKNOWN:
            default:              loaded = godot::AudioStreamMP3::load_from_buffer(buffer);       break;
        }
        if (!loaded.is_valid()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid audio buffer");
        return memnew(Audio2D(loaded, autoplay));
    }

    void Audio2D::destroy() {
        memdelete(this);
    }


    // Getters //
    godot::Ref<godot::AudioStream> Audio2D::get_stream() const {
        return stream;
    }

    godot::AudioStreamPlayer2D* Audio2D::get_player() const {
        return player;
    }


    // Misc //
    bool Audio2D::add_effect(const godot::Ref<godot::AudioEffect>& effect) {
        if (!effect.is_valid() || bus_index < 0) return false;
        auto* server = godot::AudioServer::get_singleton();
        auto current_index = server -> get_bus_index(godot::StringName(bus_name.c_str()));
        if (current_index < 0) return false;
        server -> add_bus_effect(current_index, effect);
        return true;
    }

    bool Audio2D::remove_effect(int32_t effect_index) {
        if (bus_index < 0) return false;
        auto* server = godot::AudioServer::get_singleton();
        auto current_index = server -> get_bus_index(godot::StringName(bus_name.c_str()));
        if (current_index < 0 || effect_index < 0 || effect_index >= server -> get_bus_effect_count(current_index)) return false;
        server -> remove_bus_effect(current_index, effect_index);
        return true;
    }

    godot::Ref<godot::AudioEffect> Audio2D::get_effect(int32_t effect_index) const {
        if (bus_index < 0) return nullptr;
        auto* server = godot::AudioServer::get_singleton();
        auto current_index = server -> get_bus_index(godot::StringName(bus_name.c_str()));
        if (current_index < 0 || effect_index < 0 || effect_index >= server -> get_bus_effect_count(current_index)) return nullptr;
        return server -> get_bus_effect(current_index, effect_index);
    }

    int32_t Audio2D::get_effect_count() const {
        if (bus_index < 0) return 0;
        auto* server = godot::AudioServer::get_singleton();
        auto current_index = server -> get_bus_index(godot::StringName(bus_name.c_str()));
        if (current_index < 0) return 0;
        return server -> get_bus_effect_count(current_index);
    }

    bool Audio2D::set_effect_enabled(int32_t effect_index, bool enabled) {
        if (bus_index < 0) return false;
        auto* server = godot::AudioServer::get_singleton();
        auto current_index = server -> get_bus_index(godot::StringName(bus_name.c_str()));
        if (current_index < 0 || effect_index < 0 || effect_index >= server -> get_bus_effect_count(current_index)) return false;
        server -> set_bus_effect_enabled(current_index, effect_index, enabled);
        return true;
    }

    bool Audio2D::is_effect_enabled(int32_t effect_index) const {
        if (bus_index < 0) return false;
        auto* server = godot::AudioServer::get_singleton();
        auto current_index = server -> get_bus_index(godot::StringName(bus_name.c_str()));
        if (current_index < 0 || effect_index < 0 || effect_index >= server -> get_bus_effect_count(current_index)) return false;
        return server -> is_bus_effect_enabled(current_index, effect_index);
    }
}
#endif