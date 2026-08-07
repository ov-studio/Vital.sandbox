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
        private:
            godot::Ref<godot::AudioStream> stream;
            godot::AudioStreamPlayer3D* player = nullptr;


            // Instantiators //
            Audio_3D(const godot::Ref<godot::AudioStream>& stream, bool autoplay);
            ~Audio_3D();
        public:
            // Managers //
            static Audio_3D* create(const std::string& base, const std::string& path, bool autoplay = false);
            static Audio_3D* create_from_buffer(const godot::PackedByteArray& buffer, bool autoplay = false);
            void destroy();


            // Getters //
            godot::Ref<godot::AudioStream> get_stream() const;
            godot::AudioStreamPlayer3D* get_player() const;
    };
}
#endif