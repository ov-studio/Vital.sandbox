/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: math.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Math Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>


////////////////////////
// Vital: Type: Math //
////////////////////////

namespace Vital::Type::Math {
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