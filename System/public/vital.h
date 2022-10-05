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
    static const std::string vSignature = "VitalSDK";
}

namespace Vital::Types {
    typedef struct Vector2D {
        double x = 0;
        double y = 0;
    } Vector2D;

    struct Vector3D {
        double x = 0;
        double y = 0;
        double z = 0;
    } Vector3D;
}

namespace Vital::System {
    extern unsigned int getSystemTick();
    extern unsigned int getApplicationTick();
    extern unsigned int getClientTick();
    extern bool resetClientTick();
}
namespace Sandbox {}