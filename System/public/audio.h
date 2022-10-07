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
}

namespace Vital::System::Audio::Sound {
    class create {
        private:
            FMOD::Sound* sound = nullptr;
            FMOD::Channel* channel = nullptr;
            bool is3D = false;
        public:
            // Instantiators //
            create(const std::string& path);
            ~create();

            // Checkers //
            bool isPlaying();
            bool isPaused();
            bool isLooped();
            bool isVolumeRamped();
            bool isMuted();

            // Setters //
            bool setChannelGroup(FMOD::ChannelGroup* channelGroup);
            bool setPriority(int value);
            bool setPaused(bool state);
            bool setLooped(bool state);
            bool setLoopCount(int value);
            bool setLoopPoint(Vital::Type::Audio::LoopPoint point);
            bool setPitch(float value);
            bool setFrequency(float value);
            bool setVolume(float value);
            bool setVolumeRamped(bool state);
            bool setMuted(bool state);
            bool setPosition(unsigned int value);
            bool set3DAttributes(Vital::Type::Math::Vector3D position, Vital::Type::Math::Vector3D velocity);
            bool set3DConeSettings(Vital::Type::Audio::3DConeSetting settings);
            bool set3DConeOrientation(Vital::Type::Math::Vector3D orientation);
            bool set3DDistanceFilter(Vital::Type::Audio::3DDistanceFilter filter);
            bool set3DDopplerLevel(float value);
            bool set3DLevel(float value);
            bool set3DRange(Vital::Type::Audio::3DRange range);
            bool set3DOcclusion(Vital::Type::Audio::3DOcclusion occlusion);
            bool set3DSpread(float value);
            bool setPan(float value);
            bool setMixInputLevels(Vital::Type::Audio::MixInputLevel level);
            bool setMixOutputLevels(Vital::Type::Audio::MixOutputLevel level);
            bool setMixMatrix(Vital::Type::Audio::MixMatrix matrix);

            // Getters //
            bool getChannelGroup(FMOD::ChannelGroup*& channelGroup);
            int getPriority();
            int getLoopCount();
            bool getLoopPoint(Vital::Type::Audio::LoopPoint& point);
            float getPitch();
            float getFrequency();
            float getVolume();
            float getAudibility();
            unsigned int getPosition();
            bool get3DAttributes(Vital::Type::Math::Vector3D& position, Vital::Type::Math::Vector3D& velocity);
            bool get3DConeSettings(Vital::Type::Audio::3DConeSetting& settings);
            bool get3DConeOrientation(Vital::Type::Math::Vector3D& orientation);
            bool get3DDistanceFilter(Vital::Type::Audio::3DDistanceFilter& filter);
            float get3DDopplerLevel();
            float get3DLevel();
            bool get3DRange(Vital::Type::Audio::3DRange& range);
            bool get3DOcclusion(Vital::Type::Audio::3DOcclusion& occlusion);
            float get3DSpread();
            bool getMixMatrix(Vital::Type::Audio::MixMatrix& matrix);
    
            // Utils //
            bool play();
            bool play(FMOD::ChannelGroup* channelGroup);
            bool stop();
    };
    typedef create vital_sound;
}