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
    typedef struct 3DConeSettings {
        float insideAngle;
        float outsideAngle;
        float outsideVolume;
    } 3DConeSettings;

    typedef struct 3DDistanceFilter {
        bool enable;
        float customLevel;
        float centerFrequency;
    } 3DDistanceFilter;

    typedef struct 3DRange {
        float minDistance;
        float maxDistance;
    } 3DRange;

    typedef struct 3DOcclusion {
        float directOcclusion;
        float reverbOcclusion;
    } 3DOcclusion;

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