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
        if (isErrored(FMOD::System_Create(&vSystem))) throw 0;
        if (isErrored(vSystem -> init(512, FMOD_INIT_NORMAL, 0))) throw 0;
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
}

namespace Vital::System::Audio::Sound {
    // Instantiators //
    std::map<vital_sound*, bool> vInstances;
    create::create(std::string& path) {
        vInstances.emplace(this, true);
        if (isErrored(vSystem -> createSound(path.data(), FMOD_DEFAULT, 0, &sound))) throw 0;

        // TODO: REMOVE LATER
        play();
        setPitch(2);
        setLooped(true);
        do {
                
        } while (true);
    }
    create::~create() {
        vInstances.erase(this);
        if (!sound) return;
        sound -> release();
        sound = nullptr;
        channel = nullptr;
    }

    // Checkers //
    bool create::isPlaying() {
        bool state = false;
        channel -> isPlaying(&state);
        return state;
    }
    bool create::isPaused() {
        bool state = false;
        channel -> getPaused(&state);
        return state;
    }
    bool create::isLooping() {
        bool state = false;
        FMOD_MODE mode;
        if (!isErrored(channel -> getMode(&mode))) state = mode == FMOD_LOOP_NORMAL;
        return state;
    }
    bool create::isVolumeRamped() {
        bool state = false;
        channel -> getVolumeRamp(&state);
        return state;
    }
    bool create::isMuted() {
        bool state = false;
        channel -> getMute(&state);
        return state;
    }

    // Setters //
    bool create::play() {
        return !isErrored(vSystem -> playSound(sound, nullptr, false, &channel));
    }
    bool create::play(FMOD::ChannelGroup* channelGroup) {
        return !isErrored(vSystem -> playSound(sound, channelGroup, false, &channel));
    }
    bool create::setChannelGroup(FMOD::ChannelGroup* channelGroup) {
        return !isErrored(channel -> setChannelGroup(channelGroup));
    }
    bool create::stop() {
        return !isErrored(channel -> stop());
    }
    bool create::setPaused(bool state) {
        return !isErrored(channel -> setPaused(state));
    }
    bool create::setLooped(bool state) {
        return !isErrored(channel -> setMode(FMOD_LOOP_NORMAL));
    }
    bool create::setPitch(float value) {
        return !isErrored(channel -> setPitch(value));
    }
    bool create::setVolume(float value) {
        return !isErrored(channel -> setVolume(value));
    }
    bool create::setVolumeRamped(bool state) {
        return !isErrored(channel -> setVolumeRamp(state));
    }
    bool create::setMuted(bool state) {
        return !isErrored(channel -> setMute(state));
    }
    bool create::set3DAttributes(Vital::Types::Vector3D position, Vital::Types::Vector3D velocity) {
        if (!is3D) return false;
        FMOD_VECTOR __position = {position.x, posiition.y, position.z};
        FMOD_VECTOR __velocity = {velocity.x, velocity.y, velocity.z};
        return !isErrored(channel -> set3DAttributes(__position, __velocity));
    }
    bool create::set3DConeSettings(std::array<float, float, float> coneSettings) {
        if (!is3D) return false;
        float insideConeAngle = std::max(0, std::min(360, coneSettings[0]));
        float outsideConeAngle = std::max(0, std::min(360, coneSettings[1]));
        float outsideVolume = std::max(0, std::min(360, coneSettings[2]));
        return !isErrored(channel -> set3DConeSettings(insideConeAngle, outsideConeAngle, outsideVolume));
    }
    bool create::set3DConeOrientation(std::array<float, float, float> coneOrientation) {
        if (!is3D) return false;
        FMOD_VECTOR __coneOrientation = {coneOrientation[0], coneOrientation[1], coneOrientation[2]};
        return !isErrored(channel -> set3DConeOrientation(__coneOrientation));
    }
    bool create::setPan(float value) {
        return !isErrored(channel -> setPan(value));
    }

    // Getters //
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
    bool create::get3DAttributes(Vital::Types::Vector3D& position, Vital::Types::Vector3D& velocity) {
        if (!is3D) return false;
        FMOD_VECTOR __position, __velocity;
        if (isErrored(channel -> get3DAttributes(__position, __velocity))) return false;
        position = {__position.x, __position.y, __position.z};
        velocity = {__velocity.x, __velocity.y, __velocity.z};
        return true;
    }
    bool create::get3DConeSettings(std::array<float, float, float>& coneSettings) {
        if (!is3D) return false;
        float insideConeAngle = 0, outsideConeAngle = 0, outsideVolume = 0;
        channel -> set3DConeSettings(&insideConeAngle, &outsideConeAngle, &outsideVolume);
        coneSettings = {insideConeAngle, outsideConeAngle, outsideVolume};
        return true;
    }
    bool create::get3DConeOrientation(std::array<float, float, float>& coneOrientation) {
        if (!is3D) return false;
        FMOD_VECTOR __coneOrientation;
        channel -> get3DConeOrientation(&__coneOrientation);
        coneSettings = {__coneOrientation.x, __coneOrientation.y, __coneOrientation.z};
        return true;
    }
}