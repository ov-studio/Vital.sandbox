/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: vital.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Vital Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <pch.h>


////////////
// Vital //
////////////

namespace Vital {
    static const std::string Signature = "VitalSDK";
    extern std::map<std::string, std::string> ErrorCodes;
    namespace Types {
        namespace Math {}
        namespace Audio {}
    }
    namespace System {}
    namespace Sandbox {}
}

namespace Vital::Types::Math {
    typedef struct Vector2D {
        double x = 0;
        double y = 0;
    } Vector2D;

    typedef struct Vector3D {
        double x = 0;
        double y = 0;
        double z = 0;
    } Vector3D;
}

namespace Vital::Types::Audio {
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
}

namespace Vital::System {
    extern unsigned int getSystemTick();
    extern unsigned int getApplicationTick();
    extern unsigned int getClientTick();
    extern bool resetClientTick();
}
namespace Sandbox {}