/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: audio.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Audio Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>


/////////////////////////
// Vital: Type: Audio //
/////////////////////////

namespace Vital::Type::Audio {
    typedef struct ConeSettings {
        float insideAngle;
        float outsideAngle;
        float outsideVolume;
    } ConeSettings;

    typedef struct DistanceFilter {
        bool enable;
        float customLevel;
        float centerFrequency;
    } DistanceFilter;

    typedef struct Range {
        float minDistance;
        float maxDistance;
    } Range;

    typedef struct Occlusion {
        float directOcclusion;
        float reverbOcclusion;
    } Occlusion;

    typedef struct MixInputLevels {
        float levels[];
        int count;
    } MixInputLevels;

    typedef struct MixOutputLevels {
        float frontLeft;
        float frontRight;
        float center;
        float lowFrequency;
        float surroundLeft;
        float surroundRight;
        float backLeft;
        float backRight;
    } MixOutputLevels;
}