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
        float insideAngle = 0;
        float outsideAngle = 0;
        float outsideVolume = 0;
    } ConeSettings;

    typedef struct DistanceFilter {
        bool enable = false;
        float customLevel = 0;
        float centerFrequency = 0;
    } DistanceFilter;

    typedef struct Range {
        float minDistance = 0;
        float maxDistance = 0;
    } Range;

    typedef struct Occlusion {
        float directOcclusion = 1;
        float reverbOcclusion = 10000;
    } Occlusion;
}