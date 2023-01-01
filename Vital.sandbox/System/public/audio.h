/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: audio.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
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
    // Managers //
    extern bool start();
    extern bool stop();
    extern bool update();
}


//////////////////////////////////
// Vital: System: Audio: Sound //
//////////////////////////////////

namespace Vital::System::Audio::Sound {
    class create {
        private:
            FMOD::Sound* sound = nullptr;
            FMOD::Channel* channel = nullptr;
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
            bool is3D();
    
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
            bool setPan(float value);
            bool setMixInputLevels(Vital::Type::Audio::MixInputLevel level);
            bool setMixOutputLevels(Vital::Type::Audio::MixOutputLevel level);
            bool setMixMatrix(Vital::Type::Audio::MixMatrix matrix);
            bool set3D(bool state);
            bool set3DAttributes(Vital::Type::Math::Vector3D position, Vital::Type::Math::Vector3D velocity);
            bool set3DConeSettings(Vital::Type::Audio::ConeSetting3D setting);
            bool set3DConeOrientation(Vital::Type::Math::Vector3D orientation);
            bool set3DDistanceFilter(Vital::Type::Audio::DistanceFilter3D filter);
            bool set3DDopplerLevel(float value);
            bool set3DLevel(float value);
            bool set3DRange(Vital::Type::Audio::Range3D range);
            bool set3DOcclusion(Vital::Type::Audio::Occlusion3D occlusion);
            bool set3DSpread(float value);

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
            bool getMixMatrix(Vital::Type::Audio::MixMatrix& matrix);
            bool get3DAttributes(Vital::Type::Math::Vector3D& position, Vital::Type::Math::Vector3D& velocity);
            bool get3DConeSettings(Vital::Type::Audio::ConeSetting3D& setting);
            bool get3DConeOrientation(Vital::Type::Math::Vector3D& orientation);
            bool get3DDistanceFilter(Vital::Type::Audio::DistanceFilter3D& filter);
            float get3DDopplerLevel();
            float get3DLevel();
            bool get3DRange(Vital::Type::Audio::Range3D& range);
            bool get3DOcclusion(Vital::Type::Audio::Occlusion3D& occlusion);
            float get3DSpread();
    
            // Utils //
            bool play();
            bool play(FMOD::ChannelGroup* channelGroup);
            bool stop();
    };
    typedef create vital_sound;
    extern bool isInstance(vital_sound* ref);
}