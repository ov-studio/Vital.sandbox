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
            bool set3DAttributes(Vital::Types::Math::Vector3D position, Vital::Types::Math::Vector3D velocity);
            bool set3DConeSettings(Vital::Types::Audio::ConeSettings settings);
            bool set3DConeOrientation(Vital::Types::Math::Vector3D orientation);
            bool set3DDistanceFilter(Vital::Types::Audio::DistanceFilter filter);
            bool set3DDopplerLevel(float value);
            bool setPan(float value);

            // Getters //
            float getPitch();
            float getAudibility();
            float getVolume();
            bool get3DAttributes(Vital::Types::Math::Vector3D& position, Vital::Types::Math::Vector3D& velocity);
            bool get3DConeSettings(Vital::Types::Audio::ConeSettings& settings);
            bool get3DConeOrientation(Vital::Types::Math::Vector3D& orientation);
            bool get3DDistanceFilter(Vital::Types::Audio::DistanceFilter& filter);
            float get3DDopplerLevel();
    };
    typedef create vital_sound;
}