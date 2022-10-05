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
        FMOD_MODE mode;
        if (isErrored(channel -> getMode(&mode))) throw 0;
        return mode == FMOD_LOOP_NORMAL;
    }
    bool create::isVolumeRamped() {
        bool state = false;
        if (isErrored(channel -> getVolumeRamp(&state))) throw 0;
        return state;
    }
    bool create::isMuted() {
        bool state = false;
        if (isErrored(channel -> getMute(&state))) throw 0;
        return state;
    }

    // Setters //
    bool create::play() {
        if (isErrored(vSystem -> playSound(sound, nullptr, false, &channel))) throw 0;
        return true;
    }
    bool create::play(FMOD::ChannelGroup* channelGroup) {
        if (isErrored(vSystem -> playSound(sound, channelGroup, false, &channel))) throw 0;
        return true;
    }
    bool create::setChannelGroup(FMOD::ChannelGroup* channelGroup) {
        if (isErrored(channel -> setChannelGroup(channelGroup))) throw 0;
        return true;
    }
    bool create::stop() {
        if (isErrored(channel -> stop())) throw 0;
        return true;
    }
    bool create::setPaused(bool state) {
        if (isErrored(channel -> setPaused(state))) throw 0;
        return true;
    }
    bool create::setLooped(bool state) {
        if (isErrored(channel -> setMode(FMOD_LOOP_NORMAL))) throw 0;
        return true;
    }
    bool create::setPitch(float value) {
        if (isErrored(channel -> setPitch(value))) throw 0;
        return true;
    }
    bool create::setVolume(float value) {
        if (isErrored(channel -> setVolume(value))) throw 0;
        return true;
    }
    bool create::setVolumeRamped(bool state) {
        if (isErrored(channel -> setVolumeRamp(state))) throw 0;
        return true;
    }
    bool create::setMuted(bool state) {
        if (isErrored(channel -> setMute(state))) throw 0;
        return true;
    }
    bool create::set3DAttributes(Vital::Types::Math::Vector3D position, Vital::Types::Math::Vector3D velocity) {
        if (!is3D) throw 0;
        FMOD_VECTOR __position = {static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)};
        FMOD_VECTOR __velocity = {static_cast<float>(velocity.x), static_cast<float>(velocity.y), static_cast<float>(velocity.z)};
        if (isErrored(channel -> set3DAttributes(&__position, &__velocity))) throw 0;
        return true;
    }
    bool create::set3DConeSettings(Vital::Types::Audio::ConeSettings settings) {
        if (!is3D) throw 0;
        settings.insideAngle = std::max(0.0, std::min(360.0, settings.insideAngle));
        settings.outsideAngle = std::max(0.0, std::min(360.0, settings.outsideAngle));
        settings.outsideVolume = std::max(0.0, std::min(360.0, settings.outsideVolume));
        if (isErrored(channel -> set3DConeSettings(settings.insideAngle, settings.outsideAngle, settings.outsideVolume))) throw 0;
        return true;
    }
    bool create::set3DConeOrientation(Vital::Types::Math::Vector3D orientation) {
        if (!is3D) throw 0;
        FMOD_VECTOR __orientation = {static_cast<float>(orientation.x), static_cast<float>(orientation.y), static_cast<float>(orientation.z)};
        if (isErrored(channel -> set3DConeOrientation(&__orientation))) throw 0;
        return true;
    }
    bool create::set3DDistanceFilter(Vital::Types::Audio::DistanceFilter filter) {
        if (!is3D) throw 0;
        if (isErrored(channel -> set3DDistanceFilter(filter.enable, filter.customLevel, filter.centerFrequency))) throw 0;
        return true;
    }
    bool create::set3DDopplerLevel(float value) {
        if (isErrored(channel -> set3DDopplerLevel(value))) throw 0;
        return true;
    }
    bool create::setPan(float value) {
        if (isErrored(channel -> setPan(value))) throw 0;
        return true;
    }

    // Getters //
    float create::getPitch() {
        float value = 0;
        if (isErrored(channel -> getPitch(&value))) throw 0;
        return value;
    }
    float create::getAudibility() {
        float value = 0;
        if (isErrored(channel -> getAudibility(&value))) throw 0;
        return value;
    }
    float create::getVolume() {
        float value = 0;
        if (isErrored(channel -> getVolume(&value))) throw 0;
        return value;
    }
    bool create::get3DAttributes(Vital::Types::Math::Vector3D& position, Vital::Types::Math::Vector3D& velocity) {
        if (!is3D) throw 0;
        FMOD_VECTOR __position, __velocity;
        if (isErrored(channel -> get3DAttributes(&__position, &__velocity))) throw 0;
        position = {__position.x, __position.y, __position.z};
        velocity = {__velocity.x, __velocity.y, __velocity.z};
        return true;
    }
    bool create::get3DConeSettings(Vital::Types::Audio::ConeSettings& settings) {
        if (!is3D) throw 0;
        if (isErrored(channel -> get3DConeSettings(&settings.insideAngle, &settings.outsideAngle, &settings.outsideVolume))) throw 0;
        return true;
    }
    bool create::get3DConeOrientation(Vital::Types::Math::Vector3D& orientation) {
        if (!is3D) throw 0;
        FMOD_VECTOR __orientation;
        if (isErrored(channel -> get3DConeOrientation(&__orientation))) throw 0;
        orientation = {__orientation.x, __orientation.y, __orientation.z};
        return true;
    }
    bool create::get3DDistanceFilter(Vital::Types::Audio::DistanceFilter& filter) {
        if (!is3D) throw 0;
        if (isErrored(channel -> set3DDistanceFilter(&filter.enable, &filter.customLevel, &filter.centerFrequency))) throw 0;
        return filter;
    }
    float create::get3DDopplerLevel() {
        if (!is3D) throw 0;
        float value = 0;
        if (isErrored(channel -> get3DDopplerLevel(&value))) throw 0;
        return value;
    }
}