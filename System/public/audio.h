/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: audio.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Audio System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>
#include <Vendor/fmod/include/fmod.hpp>
#include <Vendor/fmod/include/fmod_errors.h>


///////////////////////////
// Vital: System: Audio //
///////////////////////////

namespace Vital::System::Audio {
    extern bool create();
    extern bool destroy();
    extern bool update();
    extern FMOD::Sound* play(std::string& path);

    namespace Sound {
        class create {
            private:
                FMOD::Sound* sound = nullptr;
                FMOD::Channel* channel = nullptr;
            public:
                // Instantiators //
                create(std::string& path);
                bool destroy();

                // Checkers //
                bool isPlaying();

                // Setters //
                bool play();
                bool play(FMOD::ChannelGroup* channelGroup);
                bool setChannelGroup(FMOD::ChannelGroup* channelGroup);
                bool stop();
                bool setPaused(bool state);
                bool setPitch(float value);
                bool setVolume(float value);
                bool setVolumeRamp(bool state);
                bool setMute(bool state);

                bool setPan(bool state);

                // Getters //
                bool getPaused();
                float getPitch();
                float getAudibility();
                float getVolume();
                bool getVolumeRamp();
                bool getMute();
        };
        typedef create vital_sound;
    }
}