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
        float* level;
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
        float** matrix;
        int countIn;
        int countOut;
    } MixMatrix;

    typedef struct ConeSetting3D {
        float insideAngle;
        float outsideAngle;
        float outsideVolume;
    } ConeSetting3D;

    typedef struct DistanceFilter3D {
        bool enable;
        float customLevel;
        float centerFrequency;
    } DistanceFilter3D;

    typedef struct Range3D {
        float minDistance;
        float maxDistance;
    } Range3D;

    typedef struct Occlusion3D {
        float directOcclusion;
        float reverbOcclusion;
    } Occlusion3D;
}