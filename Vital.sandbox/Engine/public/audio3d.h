/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio3d.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     Desc: Audio3D Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////
// Vital: Engine: Audio3D //
/////////////////////////////

namespace Vital::Engine {
    class Audio3D : public godot::AudioStreamPlayer3D {
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
        private:
            godot::Ref<godot::AudioStream> audio;


            // Instantiators //
            Audio3D(const godot::Ref<godot::AudioStream>& audio);
            ~Audio3D() override = default;
        public:
            // Managers //
            static Audio3D* create(const std::string& base, const std::string& path);
            static Audio3D* create_from_buffer(const godot::PackedByteArray& buffer);
            void destroy();


            // Getters //
            godot::Ref<godot::AudioStream> get_audio() const;
    };
}
#endif
