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

    FMOD_RESULT F_CALLBACK channelGroupCallback(
        FMOD_CHANNELCONTROL* channelControl,
        FMOD_CHANNELCONTROL_TYPE controlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType,
        void* commandData1, void* commandData2
    ) {
        std::cout << "Callback called for " << controlType << std::endl;
        return FMOD_OK;
    }

    bool create() {
        if (vSystem) return false;
        if (!isErrored(FMOD::System_Create(&vSystem))) throw "FMOD: Failed to create system";
        if (!isErrored(vSystem -> init(512, FMOD_INIT_NORMAL, 0))) throw "FMOD: Failed to initialize system";

        // Play the sound.
        std::string url = "C:/Users/Tron/Documents/GITs/Test/Bells.mp3";
        auto sound = new Sound::create(url);
        sound -> play();

        // Create the channel group.
        //FMOD::ChannelGroup* channelGroup = nullptr;
        //if (!isErrored(vSystem->createChannelGroup("inGameSoundEffects", &channelGroup))) throw "FMOD: Failed to create in-game sound effects channel group";

        // Set a callback on the channel.
        //if (!isErrored(channel -> setCallback(&channelGroupCallback))) throw "FMOD: Failed to set callback for sound";

        bool isPlaying = false;
        do {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            //channel -> isPlaying(&isPlaying);
            update();
        } while (true);

        // Clean up.
        //channelGroup -> release();
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
            if (!isErrored(vSystem -> createSound(path.data(), FMOD_DEFAULT, 0, &sound))) throw "FMOD: Failed to create sound";
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

        void create::play() {
            if (!isErrored(vSystem -> playSound(sound, nullptr, false, &channel))) throw "FMOD: Failed to play sound";
        }
        void create::play(FMOD::ChannelGroup* channelGroup) {
            if (!isErrored(vSystem -> playSound(sound, channelGroup, false, &channel))) throw "FMOD: Failed to play sound";
        }
        void create::setChannelGroup(FMOD::ChannelGroup* channelGroup) {
            if (!isErrored(channel -> setChannelGroup(channelGroup))) throw "FMOD: Failed to set channel group on";
        }
    }
}