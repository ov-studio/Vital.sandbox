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
#include <Vendor/fmod/include/fmod.hpp>
#include <Vendor/fmod/include/fmod_errors.h>

#include <thread>


///////////////////////////
// Vital: System: Audio //
///////////////////////////

namespace Vital::System::Audio {
    FMOD::System* system = nullptr;
    FMOD_RESULT result;

    bool isValid(const std::string& message, FMOD_RESULT result) {
        if (result != FMOD_OK) {
            std::cerr << message << ": " << result << " " << FMOD_ErrorString(result) << std::endl;
            return false;
        }
        return true;
    }

    FMOD_RESULT F_CALLBACK channelGroupCallback(
        FMOD_CHANNELCONTROL* channelControl,
        FMOD_CHANNELCONTROL_TYPE controlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType,
        void* commandData1, void* commandData2
    ) {
        std::cout << "Callback called for " << controlType << std::endl;
        return FMOD_OK;
    }

    void create() {
        // Create the main system object.
        result = FMOD::System_Create(&system);
        if (!isValid("FMOD: Failed to create system object", result)) return;
        // Initialize FMOD.
        result = system -> init(512, FMOD_INIT_NORMAL, nullptr);
        if (!isValid("FMOD: Failed to initialise system object", result)) return;

        // Create the channel group.
        FMOD::ChannelGroup* channelGroup = nullptr;
        result = system->createChannelGroup("inGameSoundEffects", &channelGroup);
        if (!isValid("FMOD: Failed to create in-game sound effects channel group", result)) return;

        // Create the sound.
        FMOD::Sound* sound = nullptr;
        system->createSound("C:/Users/Tron/Documents/GITs/Test/Bells.mp3", FMOD_DEFAULT, nullptr, &sound);

        // Play the sound.
        FMOD::Channel* channel = nullptr;
        result = system->playSound(sound, nullptr, false, &channel);
        if (!isValid("FMOD: Failed to play sound", result)) return;

        // Assign the channel to the group.
        result = channel->setChannelGroup(channelGroup);
        if (!isValid("FMOD: Failed to set channel group on", result)) return;

        // Set a callback on the channel.
        channel->setCallback(&channelGroupCallback);
        if (!isValid("FMOD: Failed to set callback for sound", result)) return;

        bool isPlaying = false;
        do {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            channel->isPlaying(&isPlaying);

            system->update();
        } while (isPlaying);

        // Clean up.
        sound->release();
        channelGroup->release();
    }

    void destroy() {
        if (system) system -> release();
        system = nullptr;
    }
}