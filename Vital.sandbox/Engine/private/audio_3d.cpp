/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_3d.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     Desc: Audio3D Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/audio_3d.h>


/////////////////////////////
// Vital: Engine: Audio3D //
/////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Audio3D::Audio3D(const godot::Ref<godot::AudioStream>& stream) {
        this -> stream = stream;
        player = memnew(godot::AudioStreamPlayer3D);
        player -> set_stream(stream);
        Engine::Core::get_singleton() -> add_child(player);
    }

    Audio3D::~Audio3D() {
        if (!player) return;
        player -> queue_free();
        player = nullptr;
    }


    // Managers //
    Audio3D* Audio3D::create(const std::string& base, const std::string& path) {
        return create_from_buffer(Tool::File::read_binary(base, path));
    }

    Audio3D* Audio3D::create_from_buffer(const godot::PackedByteArray& buffer) {
        godot::Ref<godot::AudioStream> loaded;
        switch (Tool::Format::get_format(format_registry, Format::UNKNOWN, buffer)) {
            case Format::OGG:     loaded = godot::AudioStreamOggVorbis::load_from_buffer(buffer); break;
            case Format::WAV:     loaded = godot::AudioStreamWAV::load_from_buffer(buffer);       break;
            case Format::MP3:
            case Format::UNKNOWN:
            default:              loaded = godot::AudioStreamMP3::load_from_buffer(buffer);       break;
        }
        if (!loaded.is_valid()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid audio buffer");
        return memnew(Audio3D(loaded));
    }

    void Audio3D::destroy() {
        memdelete(this);
    }


    // Getters //
    godot::Ref<godot::AudioStream> Audio3D::get_stream() const {
        return stream;
    }

    godot::AudioStreamPlayer3D* Audio3D::get_player() const {
        return player;
    }
}
#endif