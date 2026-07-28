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
    Audio_2D::Audio_2D(const godot::Ref<godot::AudioStream>& stream, bool autoplay) {
        this -> stream = stream;
        player = memnew(godot::AudioStreamPlayer2D);
        player -> set_stream(stream);
        player -> set_autoplay(autoplay);
        Engine::Core::get_singleton() -> add_child(player);
        init_bus();
    }

    Audio_2D::~Audio_2D() {
        destroy_bus();
        if (!player) return;
        player -> queue_free();
        player = nullptr;
    }


    // Managers //
    Audio_2D* Audio_2D::create(const std::string& base, const std::string& path, bool autoplay) {
        return create_from_buffer(Tool::File::read_binary(base, path), autoplay);
    }

    Audio_2D* Audio_2D::create_from_buffer(const godot::PackedByteArray& buffer, bool autoplay) {
        godot::Ref<godot::AudioStream> loaded;
        switch (Tool::Format::get_format(format_registry, Format::UNKNOWN, buffer)) {
            case Format::OGG:     loaded = godot::AudioStreamOggVorbis::load_from_buffer(buffer); break;
            case Format::WAV:     loaded = godot::AudioStreamWAV::load_from_buffer(buffer);       break;
            case Format::MP3:
            case Format::UNKNOWN:
            default:              loaded = godot::AudioStreamMP3::load_from_buffer(buffer);       break;
        }
        if (!loaded.is_valid()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid audio buffer");
        return memnew(Audio_2D(loaded, autoplay));
    }

    void Audio_2D::destroy() {
        memdelete(this);
    }


    // Getters //
    godot::Ref<godot::AudioStream> Audio_2D::get_stream() const {
        return stream;
    }

    godot::AudioStreamPlayer2D* Audio_2D::get_player() const {
        return player;
    }
}
#endif