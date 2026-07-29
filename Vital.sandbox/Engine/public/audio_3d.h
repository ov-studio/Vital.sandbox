/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_3d.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Audio 3D Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/audio_bus.h>


//////////////////////////////
// Vital: Engine: Audio_3D //
//////////////////////////////

namespace Vital::Engine {
    class Audio_3D : public godot::Node3D, public Audio_Bus {
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
            godot::AudioStreamPlayer3D* player = nullptr;


            // Instantiators //
            Audio_3D(const godot::Ref<godot::AudioStream>& stream);
            ~Audio_3D();
        public:
            // Managers //
            static Audio_3D* create(const std::string& base, const std::string& path);
            static Audio_3D* create_from_buffer(const godot::PackedByteArray& buffer);
            void destroy();


            // Getters //
            godot::Ref<godot::AudioStream> get_stream() const;
            godot::AudioStreamPlayer3D* get_player() const;
    };
}
#endif