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
}

namespace Vital::System::Audio::Sound {
    class create {
        private:
            FMOD::Sound* sound = nullptr;
            FMOD::Channel* channel = nullptr;
            bool is3D = false;
        public:
            // Instantiators //
            create(std::string& path);
            ~create();

            // Checkers //
            bool isPlaying();
            bool isPaused();
            bool isLooping();
            bool isVolumeRamped();
            bool isMuted();

            // Setters //
            bool play();
            bool play(FMOD::ChannelGroup* channelGroup);
            bool setChannelGroup(FMOD::ChannelGroup* channelGroup);
            bool stop();
            bool setPaused(bool state);
            bool setLooped(bool state);
            bool setPitch(float value);
            bool setVolume(float value);
            bool setVolumeRamped(bool state);
            bool setMuted(bool state);
            bool set3DAttributes(Vital::Types::Vector3D position, Vital::Types::Vector3D velocity);
            bool set3DConeSettings(std::array<float, float, float> coneSettings);
            bool set3DConeOrientation(std::array<float, float, float> coneOrientation);
            bool setPan(float value);

            // Getters //
            float getPitch();
            float getAudibility();
            float getVolume();
            bool get3DAttributes(Vital::Types::Vector3D& position, Vital::Types::Vector3D& velocity);
            bool get3DConeSettings(std::array<float, float, float>& coneSettings);
            bool get3DConeOrientation(std::array<float, float, float>& coneOrientation);
    };
    typedef create vital_sound;
}