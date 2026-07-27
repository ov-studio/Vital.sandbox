/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio2d.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     Desc: Audio2D Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/audio2d.h>


/////////////////////////////
// Vital: Engine: Audio2D //
/////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Audio2D::Audio2D(const godot::Ref<godot::AudioStream>& audio) {
        this -> audio = audio;
        set_stream(audio);
        Engine::Core::get_singleton() -> add_child(this);
    }


    // Managers //
    Audio2D* Audio2D::create(const std::string& base, const std::string& path) {
        return create_from_buffer(Tool::File::read_binary(base, path));
    }

    Audio2D* Audio2D::create_from_buffer(const godot::PackedByteArray& buffer) {
        godot::Ref<godot::AudioStream> loaded;
        switch (Tool::Format::get_format(format_registry, Format::UNKNOWN, buffer)) {
            case Format::OGG:     loaded = godot::AudioStreamOggVorbis::load_from_buffer(buffer); break;
            case Format::WAV:     loaded = godot::AudioStreamWAV::load_from_buffer(buffer);       break;
            case Format::MP3:
            case Format::UNKNOWN:
            default:              loaded = godot::AudioStreamMP3::load_from_buffer(buffer);       break;
        }
        if (!loaded.is_valid()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid audio buffer");
        return memnew(Audio2D(loaded));
    }

    void Audio2D::destroy() {
        queue_free();
    }


    // Getters //
    godot::Ref<godot::AudioStream> Audio2D::get_audio() const {
        return audio;
    }
}
#endif
