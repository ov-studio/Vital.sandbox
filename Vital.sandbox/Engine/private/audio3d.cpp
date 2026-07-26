/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio3d.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     Desc: 3D Audio Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/audio3d.h>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/audio_stream_ogg_vorbis.hpp>
#include <godot_cpp/classes/audio_stream_mp3.hpp>


///////////////////////////////
// Vital: Engine: Audio3D //
///////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Audio3D::Audio3D(const std::string& reference) {
        if (!reference.empty()) {
            reference_key = reference;
            reference_cache.emplace(reference_key, this);
            heartbeat();
        }
    }

    Audio3D::~Audio3D() {
        if (reference_key.empty()) return;
        reference_cache.erase(reference_key);
    }


    // Managers //
    void Audio3D::destroy() {
        memdelete(this);
    }

    void Audio3D::heartbeat() {
        reference_tick = Tool::get_tick();
    }

    void Audio3D::flush() {
        auto tick = Tool::get_tick();
        std::vector<std::string> expired;
        for (const auto& reference_cache : reference_cache) {
            if (tick - reference_cache.second -> reference_tick > flush_interval) {
                expired.push_back(reference_cache.first);
            }
        }
        for (const auto& key : expired) reference_cache[key] -> destroy();
    }


    // Getters //
    Audio3D* Audio3D::get_from_reference(const std::string& reference) {
        auto it = reference_cache.find(reference);
        return it != reference_cache.end() ? it -> second : nullptr;
    }

    godot::Ref<godot::AudioStream> Audio3D::get_stream() const {
        return stream;
    }


    // Misc //
    Audio3D* Audio3D::create(const std::string& base, const std::string& path, const std::string& reference) {
        return create_from_buffer(Tool::File::read_binary(base, path), reference);
    }

    Audio3D* Audio3D::create_from_buffer(const godot::PackedByteArray& buffer, const std::string& reference) {
        godot::Ref<godot::AudioStream> loaded;
        switch (Tool::Format::get_format(format_registry, Format::UNKNOWN, buffer)) {
            case Format::OGG: loaded = godot::AudioStreamOggVorbis::load_from_buffer(buffer); break;
            case Format::WAV: loaded = godot::AudioStreamWAV::load_from_buffer(buffer);        break;
            case Format::MP3:
            case Format::UNKNOWN:
            default:          loaded = godot::AudioStreamMP3::load_from_buffer(buffer);        break;
        }
        if (!loaded.is_valid()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid audio buffer");

        auto instance = memnew(Audio3D(reference));
        instance -> stream = loaded;
        instance -> set_stream(loaded);
        return instance;
    }
}
#endif
