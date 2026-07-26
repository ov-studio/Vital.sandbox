/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio2d.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     Desc: 2D Audio Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>
#include <godot_cpp/classes/audio_stream_player2d.hpp>
#include <godot_cpp/classes/audio_stream.hpp>


///////////////////////////////
// Vital: Engine: Audio2D //
///////////////////////////////

namespace Vital::Engine {
    class Audio2D : public godot::AudioStreamPlayer2D {
        public:
            enum class Format {
                WAV,
                OGG,
                MP3,
                UNKNOWN
            };

            // Note: MP3 has no reliable magic-byte signature (only ID3-tagged
            // files start with "ID3"; raw MPEG frames don't). Buffers that
            // match neither OGG nor WAV fall back to MP3 in create_from_buffer.
            inline static const std::vector<Tool::Format::Descriptor<Format>> format_registry = {
                { Format::OGG, "ogg", { 0x4F, 0x67, 0x67, 0x53 } }, // "OggS"
                { Format::WAV, "wav", { 0x52, 0x49, 0x46, 0x46 } }, // "RIFF"
                { Format::MP3, "mp3", { 0x49, 0x44, 0x33 }       }  // "ID3"
            };

            inline static const unsigned int flush_interval = 10000;
        protected:
            uint64_t reference_tick = 0;
            std::string reference_key = "";
            godot::Ref<godot::AudioStream> stream;
            inline static std::unordered_map<std::string, Audio2D*> reference_cache = {};


            // Instantiators //
            Audio2D(const std::string& reference = "");
            ~Audio2D();
        public:
            // Managers //
            void destroy();
            void heartbeat();
            static void flush();


            // Getters //
            static Audio2D* get_from_reference(const std::string& reference);
            godot::Ref<godot::AudioStream> get_stream() const;


            // Misc //
            static Audio2D* create(const std::string& base, const std::string& path, const std::string& reference = "");
            static Audio2D* create_from_buffer(const godot::PackedByteArray& buffer, const std::string& reference = "");
    };
}
#endif
