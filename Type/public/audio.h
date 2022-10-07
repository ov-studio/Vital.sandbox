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
    typedef struct LoopPoint {
        unsigned int start;
        unsigned int end;
    } LoopPoint;

    typedef struct MixInputLevel {
        float level[];
        int count;
    } MixInputLevel;

    typedef struct MixOutputLevel {
        float frontLeft;
        float frontRight;
        float center;
        float lowFrequency;
        float surroundLeft;
        float surroundRight;
        float backLeft;
        float backRight;
    } MixOutputLevel;

    typedef struct MixMatrix {
        float matrix[][];
        int countIn;
        int countOut;
    } MixMatrix;

    typedef struct 3DConeSetting {
        float insideAngle;
        float outsideAngle;
        float outsideVolume;
    } 3DConeSetting;

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
}