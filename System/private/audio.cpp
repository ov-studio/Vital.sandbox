/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: audio.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Audio System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/audio.h>


///////////////////////////
// Vital: System: Audio //
///////////////////////////

namespace Vital::System::Audio {
    FMOD::System* vSystem = nullptr;
    bool isErrored(FMOD_RESULT result) { return result != FMOD_OK; }

    /*
    * TODO: ADD THIS UNDER CHANNEL GROUP..
    FMOD_RESULT F_CALLBACK channelGroupCallback(
        FMOD_CHANNELCONTROL* channelControl,
        FMOD_CHANNELCONTROL_TYPE controlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType,
        void* commandData1, void* commandData2
    ) {
        std::cout << "Callback called for " << controlType << std::endl;
        return FMOD_OK;
    }

    // Create the channel group.
    FMOD::ChannelGroup* channelGroup = nullptr;
    if (!isErrored(vSystem->createChannelGroup("inGameSoundEffects", &channelGroup))) throw "FMOD: Failed to create in-game sound effects channel group";

    // Set a callback on the channel.
    if (!isErrored(channel -> setCallback(&channelGroupCallback))) throw "FMOD: Failed to set callback for sound";

    channelGroup -> release();
    */

    bool create() {
        if (vSystem) return false;
        if (isErrored(FMOD::System_Create(&vSystem))) throw "FMOD: Failed to create system";
        if (isErrored(vSystem -> init(512, FMOD_INIT_NORMAL, 0))) throw "FMOD: Failed to initialize system";

        std::string url = "C:/Users/Tron/Documents/GITs/Test/Bells.mp3";
        auto sound = new Sound::create(url);
        sound -> play();
        do {
            update();
        } while (true);
        return true;
    }

    bool destroy() {
        if (!vSystem) return false;
        vSystem -> release();
        vSystem = nullptr;
        return true;
    }

    bool update() {
        vSystem -> update();
        return true;
    }

    namespace Sound {
        // Instantiators //
        std::map<vital_sound*, bool> vInstances;
        create::create(std::string& path) {
            if (isErrored(vSystem -> createSound(path.data(), FMOD_DEFAULT, 0, &sound))) throw "FMOD: Failed to create sound";
            vInstances.emplace(this, true);
        }
        bool create::destroy() {
            if (!sound) return false;
            vInstances.erase(this);
            sound -> release();
            sound = nullptr;
            channel = nullptr;
            return true;
        }

        bool create::play() {
            return !isErrored(vSystem -> playSound(sound, nullptr, false, &channel));
        }
        bool create::play(FMOD::ChannelGroup* channelGroup) {
            return !isErrored(vSystem -> playSound(sound, channelGroup, false, &channel));
        }
        bool create::setChannelGroup(FMOD::ChannelGroup* channelGroup) {
            return !isErrored(channel -> setChannelGroup(channelGroup));
        }

        bool create::isPlaying() {
            bool state = false;
            channel -> isPlaying(&state);
            return state;
        }

        bool create::getPaused() {
            bool state = false;
            channel->getPaused(&state);
            return state;
        }
        float create::getPitch() {
            float value = 0;
            channel -> getPitch(&value);
            return value;
        }
        float create::getAudibility() {
            float value = 0;
            channel -> getAudibility(&value);
            return value;
        }
        float create::getVolume() {
            float value = 0;
            channel -> getVolume(&value);
            return value;
        }
        bool create::getVolumeRamp() {
            bool state = false;
            channel -> getVolumeRamp(&state);
            return state;
        }
        bool create::getMute() {
            bool state = false;
            channel -> getMute(&state);
            return state;
        }

        bool create::stop() {
            return !isErrored(channel -> stop());
        }
        bool create::setPaused(bool state) {
            return !isErrored(channel -> setPaused(state));
        }
        bool create::setPitch(float value) {
            return !isErrored(channel -> setPitch(value));
        }
        bool create::setVolume(float value) {
            return !isErrored(channel -> setVolume(value));
        }
        bool create::setVolumeRamp(bool state) {
            return !isErrored(channel -> setVolumeRamp(state));
        }
        bool create::setMute(bool state) {
            return !isErrored(channel -> setMute(state));
        }
    }
}