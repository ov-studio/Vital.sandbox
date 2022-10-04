/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: audio.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Audio System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>
#include <Vendor/fmod/include/fmod.hpp>
#include <Vendor/fmod/include/fmod_errors.h>


///////////////////////////
// Vital: System: Audio //
///////////////////////////

namespace Vital::System::Audio {
    typedef vital_sound FMOD::Sound;
    extern bool create();
    extern bool destroy();
    extern bool update();
}