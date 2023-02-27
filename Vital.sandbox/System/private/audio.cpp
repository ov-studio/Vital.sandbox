/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: audio.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
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

namespace Vital::System::Audio::Sound {
    // Instantiators //
    std::map<vital_sound*, bool> instance;
    bool isInstance(vital_sound* ref) { return instance.find(ref) != instance.end(); }
}

namespace Vital::System::Audio {
    // Handlers //
    FMOD::System* system = nullptr;
    bool isErrored(FMOD_RESULT result) { return result != FMOD_OK; }

    // Managers //
    bool start() {
        if (system) return false;
        if (isErrored(FMOD::System_Create(&system))) return false;
        if (isErrored(system -> init(512, FMOD_INIT_NORMAL, 0))) return false;
        return true;
    }
    bool stop() {
        if (!system) return false;
        for (const auto& i : Vital::System::Audio::Sound::instance) {
            delete i.first;
        }
        system -> release();
        system = nullptr;
        return true;
    }
    bool update() {
        if (!system) return false;
        system -> update();
        return true;
    }
}


//////////////////////////////////
// Vital: System: Audio: Sound //
//////////////////////////////////

namespace Vital::System::Audio::Sound {
    // Instantiators //
    create::create(const std::string& path) {
        instance.emplace(this, true);
        if (isErrored(system -> createSound(path.data(), FMOD_DEFAULT, 0, &sound))) throw ErrorCode["request-failed"];
    }
    create::~create() {
        instance.erase(this);
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
    bool create::isLooped() {
        FMOD_MODE mode;
        if (isErrored(channel -> getMode(&mode))) throw ErrorCode["request-failed"];
        return mode & FMOD_LOOP_NORMAL;
    }
    bool create::isVolumeRamped() {
        bool state = false;
        if (isErrored(channel -> getVolumeRamp(&state))) throw ErrorCode["request-failed"];
        return state;
    }
    bool create::isMuted() {
        bool state = false;
        if (isErrored(channel -> getMute(&state))) throw ErrorCode["request-failed"];
        return state;
    }
    bool create::is3D() {
        FMOD_MODE mode;
        if (isErrored(channel -> getMode(&mode))) throw ErrorCode["request-failed"];
        return mode & FMOD_3D;
    }

    // Setters //
    bool create::setChannelGroup(FMOD::ChannelGroup* channelGroup) {
        if (isErrored(channel -> setChannelGroup(channelGroup))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setPriority(int value) {
        if (isErrored(channel -> setPriority(value))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setPaused(bool state) {
        if (isErrored(channel -> setPaused(state))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setLooped(bool state) {
        if (isErrored(channel -> setMode(FMOD_LOOP_NORMAL))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setLoopCount(int value) {
        if (!isLooped()) throw ErrorCode["sound-invalid-loop"];
        if (isErrored(channel -> setLoopCount(value))) throw ErrorCode["request-failed"];
        return true; 
    }
    bool create::setLoopPoint(Vital::Type::Audio::LoopPoint point) {
        if (!isLooped()) throw ErrorCode["sound-invalid-loop"];
        if (isErrored(channel -> setLoopPoints(point.start, FMOD_TIMEUNIT_MS, point.end, FMOD_TIMEUNIT_MS))) throw ErrorCode["request-failed"];
        return true; 
    }
    bool create::setPitch(float value) {
        if (isErrored(channel -> setPitch(value))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setFrequency(float value) {
        if (isErrored(channel -> setFrequency(value))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setVolume(float value) {
        if (isErrored(channel -> setVolume(value))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setVolumeRamped(bool state) {
        if (isErrored(channel -> setVolumeRamp(state))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setMuted(bool state) {
        if (isErrored(channel -> setMute(state))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setPosition(unsigned int value) {
        if (isErrored(channel -> setPosition(value, FMOD_TIMEUNIT_MS))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setPan(float value) {
        if (isErrored(channel -> setPan(value))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setMixInputLevels(Vital::Type::Audio::MixInputLevel level) {
        if (isErrored(channel -> setMixLevelsInput(level.level, level.count))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setMixOutputLevels(Vital::Type::Audio::MixOutputLevel level) {
        if (isErrored(channel -> setMixLevelsOutput(level.frontLeft, level.frontRight, level.center, level.lowFrequency, level.surroundLeft, level.surroundRight, level.backLeft, level.backRight))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::setMixMatrix(Vital::Type::Audio::MixMatrix matrix) {
        if (isErrored(channel -> setMixMatrix(matrix.matrix, matrix.countOut, matrix.countIn))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::set3D(bool state) {
        if (isErrored(channel -> setMode(state ? FMOD_3D : FMOD_2D))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::set3DAttributes(Vital::Type::Math::Vector3D position, Vital::Type::Math::Vector3D velocity) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        FMOD_VECTOR __position = {static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)};
        FMOD_VECTOR __velocity = {static_cast<float>(velocity.x), static_cast<float>(velocity.y), static_cast<float>(velocity.z)};
        if (isErrored(channel -> set3DAttributes(&__position, &__velocity))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::set3DConeSettings(Vital::Type::Audio::ConeSetting3D setting) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        if (isErrored(channel -> set3DConeSettings(setting.insideAngle, setting.outsideAngle, setting.outsideVolume))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::set3DConeOrientation(Vital::Type::Math::Vector3D orientation) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        FMOD_VECTOR __orientation = {static_cast<float>(orientation.x), static_cast<float>(orientation.y), static_cast<float>(orientation.z)};
        if (isErrored(channel -> set3DConeOrientation(&__orientation))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::set3DDistanceFilter(Vital::Type::Audio::DistanceFilter3D filter) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        if (isErrored(channel -> set3DDistanceFilter(filter.isEnabled, filter.customLevel, filter.centerFrequency))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::set3DDopplerLevel(float value) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        if (isErrored(channel -> set3DDopplerLevel(value))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::set3DLevel(float value) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        if (isErrored(channel -> set3DLevel(value))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::set3DRange(Vital::Type::Audio::Range3D range) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        if (isErrored(channel -> set3DMinMaxDistance(range.minDistance, range.maxDistance))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::set3DOcclusion(Vital::Type::Audio::Occlusion3D occlusion) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        if (isErrored(channel -> set3DOcclusion(occlusion.directOcclusion, occlusion.reverbOcclusion))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::set3DSpread(float value) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        if (isErrored(channel -> set3DSpread(value))) throw ErrorCode["request-failed"];
        return true;
    }

    // Getters //
    bool create::getChannelGroup(FMOD::ChannelGroup*& channelGroup) {
        if (isErrored(channel -> getChannelGroup(&channelGroup))) throw ErrorCode["request-failed"];
        return true;
    }
    int create::getPriority() {
        int value;
        if (isErrored(channel -> getPriority(&value))) throw ErrorCode["request-failed"];
        return value;
    }
    int create::getLoopCount() {
        if (!isLooped()) throw ErrorCode["sound-invalid-loop"];
        int value;
        if (isErrored(channel -> getLoopCount(&value))) throw ErrorCode["request-failed"];
        return value; 
    }
    bool create::getLoopPoint(Vital::Type::Audio::LoopPoint& point) {
        if (!isLooped()) throw ErrorCode["sound-invalid-loop"];
        if (isErrored(channel -> getLoopPoints(&point.start, FMOD_TIMEUNIT_MS, &point.end, FMOD_TIMEUNIT_MS))) throw ErrorCode["request-failed"];
        return true; 
    }
    float create::getPitch() {
        float value;
        if (isErrored(channel -> getPitch(&value))) throw ErrorCode["request-failed"];
        return value;
    }
    float create::getFrequency() {
        float value;
        if (isErrored(channel -> getFrequency(&value))) throw ErrorCode["request-failed"];
        return value;
    }
    float create::getVolume() {
        float value;
        if (isErrored(channel -> getVolume(&value))) throw ErrorCode["request-failed"];
        return value;
    }
    float create::getAudibility() {
        float value;
        if (isErrored(channel -> getAudibility(&value))) throw ErrorCode["request-failed"];
        return value;
    }
    unsigned int create::getPosition() {
        unsigned int value;
        if (isErrored(channel -> getPosition(&value, FMOD_TIMEUNIT_MS))) throw ErrorCode["request-failed"];
        return value;
    }
    bool create::getMixMatrix(Vital::Type::Audio::MixMatrix& matrix) {
        if (isErrored(channel -> getMixMatrix(matrix.matrix, &matrix.countOut, &matrix.countIn))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::get3DAttributes(Vital::Type::Math::Vector3D& position, Vital::Type::Math::Vector3D& velocity) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        FMOD_VECTOR __position, __velocity;
        if (isErrored(channel -> get3DAttributes(&__position, &__velocity))) throw ErrorCode["request-failed"];
        position = {__position.x, __position.y, __position.z};
        velocity = {__velocity.x, __velocity.y, __velocity.z};
        return true;
    }
    bool create::get3DConeSettings(Vital::Type::Audio::ConeSetting3D& setting) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        if (isErrored(channel -> get3DConeSettings(&setting.insideAngle, &setting.outsideAngle, &setting.outsideVolume))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::get3DConeOrientation(Vital::Type::Math::Vector3D& orientation) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        FMOD_VECTOR __orientation;
        if (isErrored(channel -> get3DConeOrientation(&__orientation))) throw ErrorCode["request-failed"];
        orientation = {__orientation.x, __orientation.y, __orientation.z};
        return true;
    }
    bool create::get3DDistanceFilter(Vital::Type::Audio::DistanceFilter3D& filter) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        if (isErrored(channel -> set3DDistanceFilter(filter.isEnabled, filter.customLevel, filter.centerFrequency))) throw ErrorCode["request-failed"];
        return true;
    }
    float create::get3DDopplerLevel() {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        float value;
        if (isErrored(channel -> get3DDopplerLevel(&value))) throw ErrorCode["request-failed"];
        return value;
    }
    float create::get3DLevel() {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        float value;
        if (isErrored(channel -> get3DDopplerLevel(&value))) throw ErrorCode["request-failed"];
        return value;
    }
    bool create::get3DRange(Vital::Type::Audio::Range3D& range) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        if (isErrored(channel -> get3DMinMaxDistance(&range.minDistance, &range.maxDistance))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::get3DOcclusion(Vital::Type::Audio::Occlusion3D& occlusion) {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        if (isErrored(channel -> get3DOcclusion(&occlusion.directOcclusion, &occlusion.reverbOcclusion))) throw ErrorCode["request-failed"];
        return true;
    }
    float create::get3DSpread() {
        if (!is3D()) throw ErrorCode["sound-invalid-3d"];
        float value;
        if (isErrored(channel -> get3DSpread(&value))) throw ErrorCode["request-failed"];
        return value;
    }

    // Utils //
    bool create::play() {
        if (isErrored(system -> playSound(sound, nullptr, false, &channel))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::play(FMOD::ChannelGroup* channelGroup) {
        if (isErrored(system -> playSound(sound, channelGroup, false, &channel))) throw ErrorCode["request-failed"];
        return true;
    }
    bool create::stop() {
        if (isErrored(channel -> stop())) throw ErrorCode["request-failed"];
        return true;
    }
}