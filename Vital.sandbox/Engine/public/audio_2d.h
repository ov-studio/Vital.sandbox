/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_2d.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     Desc: Audio 2D Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


//////////////////////////////
// Vital: Engine: Audio_2D //
//////////////////////////////

namespace Vital::Engine {
    class Audio2D : public godot::Node2D {
        public:
            enum class Format {
                OGG,
                WAV,
                MP3,
                UNKNOWN
            };

            inline static const std::vector<Tool::Format::Descriptor<Format>> format_registry = {
                { Format::OGG, "ogg", { 0x4F, 0x67, 0x67, 0x53 } },
                { Format::WAV, "wav", { 0x52, 0x49, 0x46, 0x46 } },
                { Format::MP3, "mp3", { 0x49, 0x44, 0x33 }       }
            };
        private:
            godot::Ref<godot::AudioStream> stream;
            godot::AudioStreamPlayer2D* player = nullptr;


            // Instantiators //
            Audio2D(const godot::Ref<godot::AudioStream>& stream, bool autoplay);
            ~Audio2D();
        public:
            // Managers //
            static Audio2D* create(const std::string& base, const std::string& path, bool autoplay = false);
            static Audio2D* create_from_buffer(const godot::PackedByteArray& buffer, bool autoplay = false);
            void destroy();


            // Getters //
            godot::Ref<godot::AudioStream> get_stream() const;
            godot::AudioStreamPlayer2D* get_player() const;
    };
}
#endif